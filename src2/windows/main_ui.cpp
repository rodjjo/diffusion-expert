#include <map>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include "windows/progress_ui.h"
#include "misc/dialogs.h"
#include "misc/config.h"
#include "misc/utils.h"

#include "windows/main_ui.h"


namespace dfe
{
    namespace
    {
        bool stopped = false;
        const std::list<event_id_t> main_window_events = {
            event_main_menu_clicked,
            event_main_menu_file_new_art,
            event_main_menu_file_open,
            event_main_menu_file_open_layer,
            event_main_menu_file_close,
            event_main_menu_edit_settings,
            event_layer_count_changed,
            event_main_menu_exit
        };
    } // namespace

    MainWindow::MainWindow() : Fl_Menu_Window(640, 480, "Diffusion Expert"), SubscriberThis(main_window_events)
    {
        auto wnd_ = this;

        { // menu
            menuPanel_ = new Fl_Group(0, 20, this->w(), 20);
            menuPanel_->end();
            menuPanel_->box(FL_BORDER_BOX);
            menuPanel_->begin();
            menu_ = new MainMenu(this->w(), 20);
            menuPanel_->end();
            menu_->addItem(event_main_menu_file_new_art, "", "File/New Art", "^n", 0, xpm::img_24x24_new);
            menu_->addItem(event_main_menu_file_open, "", "File/Open", "^o", 0, xpm::img_24x24_open);
            menu_->addItem(event_main_menu_file_open_layer, "", "File/Open as Layer", "^l", 0, xpm::img_24x24_open_layer);
            menu_->addItem(event_main_menu_file_close, "", "File/Close", "^x", 0, xpm::img_24x24_close);
            menu_->addItem(event_main_menu_exit, "", "File/Exit", "", 0, xpm::img_24x24_exit);
            menu_->addItem(event_main_menu_edit_settings, "", "Edit/Settings", "", 0, xpm::img_24x24_settings);
        } // menu

        { // image panels
            image_ = new ImagePanel(0, 0, 1, 1, "MainWindowImagePanel");
            layers_ = new Fl_Select_Browser(0, 0, 1, 1);
            layers_->callback(layer_cb, this);
            removeLayer_.reset(new Button(xpm::image(xpm::img_24x24_remove_layer), [this] {
                remove_selected_layer();
            }));
            removeAllLayers_.reset(new Button(xpm::image(xpm::img_24x24_remove_all_layers), [this] {
                clear_layers();
            }));
            removeLayer_->tooltip("Remove selected layer");
            removeAllLayers_->tooltip("Remove all the layers");
            removeLayer_->size(28, 28);
            removeAllLayers_->size(28, 28);
        }

        wnd_->end();
        wnd_->position(Fl::w() / 2 - wnd_->w() / 2, Fl::h() / 2 - wnd_->h() / 2);
        wnd_->size_range(860, 480);
        wnd_->show();

#ifdef _WIN32
        auto native_wnd = find_current_thread_window(wnd_->label());
        if (native_wnd) {
            /*
            FLTK does not handle mouse wheel event weel.
            I'm hacking the window proc so.
            */
            image_->hack_window_proc(native_wnd);
        }
#endif
        alignComponents();
    }

    MainWindow::~MainWindow(){
    };

    int MainWindow::dfe_run()
    {
        puts("Starting user interface...");
        Fl::scheme("gtk+");
        MainWindow *wnd = new MainWindow();
        while (!stopped)
        {
            Fl::wait(0.33);
            if (!wnd->shown())
            {
                break;
            }
        }
        return 0;
    }
    void MainWindow::layer_cb(Fl_Widget* widget, void *cbdata) {
        static_cast<MainWindow *>(cbdata)->layer_cb(widget);
    }

    void MainWindow::layer_cb(Fl_Widget* widget) {
        if (widget == layers_) {
            if (in_layer_callback_) {
                return;
            }
            in_layer_callback_ = true;
            int idx = layers_->value();
            if (idx > 0)  {
                // do something 
            } 
            layers_->deselect();
            layers_->select(idx);
            in_layer_callback_ = false;
        }
    }

    void MainWindow::dfe_stop()
    {
        dfe_hideProgress();
        stopped = true;
    }

    void MainWindow::dfe_close()
    {
        stopped = true;
    }

    void MainWindow::dfe_showProgress()
    {
        show_progress_window();
    }

    void MainWindow::dfe_hideProgress()
    {
        hide_progress_window();
    }

    void MainWindow::dfe_show_error(const char *message) {
        fl_alert(message);
    }

    void MainWindow::alignComponents() {
        menuPanel_->position(0, 0);
        int w = this->w();
        int h = this->h();
        menuPanel_->size(w, menu_->h());
        menu_->position(0, 0);
        menu_->size(w, menuPanel_->h());
        if (image_->view_settings()->layer_count() > 1) {
            removeAllLayers_->position(w - removeAllLayers_->w() - 1, menuPanel_->h() + 1);
            removeLayer_->position(removeAllLayers_->x() - removeLayer_->w() - 2,  removeAllLayers_->y());
            layers_->size(100, h - (menuPanel_->h() + removeAllLayers_->h()) - 2);
            layers_->position(w - 1 - layers_->w(), removeAllLayers_->h() + removeAllLayers_->y() + 1);
            image_->size(w - 5 - layers_->w(), h - menuPanel_->h() - 2);
            layers_->show();
            removeLayer_->show();
            removeAllLayers_->show();
        } else {
            layers_->hide();
            removeLayer_->hide();
            removeAllLayers_->hide();
            image_->size(w - 2, h - menuPanel_->h() - 2);
        }
        image_->position(1, menuPanel_->h() + 1);
    }

    void MainWindow::resize(int x, int y, int w, int h)
    {
        Fl_Menu_Window::resize(x, y, w, h);
        alignComponents();
    }

    int MainWindow::handle(int event)
    {
        switch (event)
        {
        case FL_KEYUP:
        {
            if (Fl::event_key() == FL_Escape)
            {
                return 1;
            }
        }
        break;
        case FL_KEYDOWN:
        {
            if (Fl::event_key() == FL_Escape)
            {
                return 1;
            }
        }
        break;
        }

        return Fl_Menu_Window::handle(event);
    }

    void MainWindow::dfe_handle_event(void *sender, event_id_t id, void *data)
    {
        switch (id)
        {
        case event_main_menu_clicked:
            break;
        case event_main_menu_file_new_art:
            break;
        case event_main_menu_file_open:
            choose_file_and_open(true);
            break;
        case event_main_menu_file_open_layer:
            choose_file_and_open(false);
            break;
        case event_main_menu_file_close:
            clear_layers();
            break;
        case event_layer_count_changed:
            if (sender == image_) {
                update_layer_list();
            }
            break;
        case event_main_menu_edit_settings:
            break;
        
        case event_main_menu_exit:
            this->hide();
            break;
        }
    }

    void MainWindow::choose_file_and_open(bool clear_layers) {
        std::string current_dir = get_config()->last_open_directory("main_window_picture");
        std::string result = choose_image_to_open_fl(&current_dir);
        if (!result.empty()) {
            auto dir = filepath_dir(result);
            get_config()->last_open_directory("main_window_picture", dir.c_str());
            if (clear_layers) {
                image_->view_settings()->clear_layers();
            }
            image_->view_settings()->add_layer(result.c_str());
        }
    }

    void MainWindow::update_layer_list() {
        alignComponents();
        layers_->clear();
        for (size_t i = 0; i < image_->view_settings()->layer_count(); i++) {
            layers_->add(image_->view_settings()->at(i)->name());
        }
    }

    void MainWindow::remove_selected_layer() {
        if (ask("Do you want to remove the selected layer ?")) {
            image_->view_settings()->remove_layer(layers_->value());
        }
    }

    void MainWindow::clear_layers() {
        const char *message = image_->view_settings()->layer_count() > 1 ? "Do you want to close all the layers ?" : "Do you want to close the image ?";
        if (ask(message)) {
            image_->view_settings()->clear_layers();
        }
    }

} // namespace dfe
