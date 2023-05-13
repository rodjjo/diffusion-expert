import os
import re

import torch
from PIL import Image
import numpy as np

from models.paths import DEEPBOORU_DIR
from external.deepbooru_model import DeepDanbooruModel
from utils.settings import get_setting

re_special = re.compile(r'([\\()])')

usefp16 = {
    True: torch.float16,
    False: torch.float32
}

def resize_image(im, width, height):
    ratio = width / height
    src_ratio = im.width / im.height

    src_w = width if ratio < src_ratio else im.width * height // im.height
    src_h = height if ratio >= src_ratio else im.height * width // im.width

    resized = im.resize((width, width), resample=Image.LANCZOS)
    res = Image.new("RGB", (width, height))
    res.paste(resized, box=(width // 2 - src_w // 2, height // 2 - src_h // 2))

    if ratio < src_ratio:
        fill_height = height // 2 - src_h // 2
        res.paste(resized.resize((width, fill_height), box=(0, 0, width, 0)), box=(0, 0))
        res.paste(resized.resize((width, fill_height), box=(0, resized.height, width, resized.height)), box=(0, fill_height + src_h))
    elif ratio > src_ratio:
        fill_width = width // 2 - src_w // 2
        res.paste(resized.resize((fill_width, height), box=(0, 0, 0, height)), box=(0, 0))
        res.paste(resized.resize((fill_width, height), box=(resized.width, 0, resized.width, height)), box=(fill_width + src_w, 0))
    return res

def torch_gc():
    if torch.cuda.is_available():
        with torch.cuda.device('cuda'):
            torch.cuda.empty_cache()
            torch.cuda.ipc_collect()

class DeepDanbooru:
    def __init__(self):
        self.model = None

    def load(self):
        if self.model is not None:
            return
        
        model_path = os.path.join(DEEPBOORU_DIR, 'model-resnet_custom_v3.pt')


        self.model = DeepDanbooruModel()
        self.model.load_state_dict(torch.load(model_path, map_location="cpu"))

        self.model.eval()
        self.model.to(get_setting('device', 'cpu'), torch.float32)

    def start(self):
        self.load()
        self.model.to(get_setting('device', 'cpu'))

    def stop(self):
        self.model.to(get_setting('device', 'cpu'))
        torch_gc()

    def tag(self, pil_image, threshold=0.5):
        self.start()
        res = self.tag_multi(pil_image, threshold)
        self.stop()

        return res

    def tag_multi(self, pil_image, threshold):
        threshold = threshold
        use_spaces = False
        use_escape = True
        alpha_sort = True
        include_ranks = False
        exclude_tags = ""

        pic = resize_image(pil_image.convert("RGB"), 512, 512)
        a = np.expand_dims(np.array(pic, dtype=np.float32), 0) / 255

        with torch.no_grad(), torch.autocast(get_setting('device', 'cpu')):
            x = torch.from_numpy(a).to(get_setting('device', 'cpu'))
            y = self.model(x)[0].detach().cpu().numpy()

        probability_dict = {}

        for tag, probability in zip(self.model.tags, y):
            if probability < threshold:
                continue

            if tag.startswith("rating:"):
                continue

            probability_dict[tag] = probability

        if alpha_sort:
            tags = sorted(probability_dict)
        else:
            tags = [tag for tag, _ in sorted(probability_dict.items(), key=lambda x: -x[1])]

        res = []

        filtertags = set([x.strip().replace(' ', '_') for x in exclude_tags.split(",")])

        for tag in [x for x in tags if x not in filtertags]:
            probability = probability_dict[tag]
            tag_outformat = tag
            if use_spaces:
                tag_outformat = tag_outformat.replace('_', ' ')
            if use_escape:
                tag_outformat = re.sub(re_special, r'\\\1', tag_outformat)
            if include_ranks:
                tag_outformat = f"({tag_outformat}:{probability:.3f})"

            res.append(tag_outformat)

        return ", ".join(res)


model = DeepDanbooru()

def get_deepbooru_tags(pil_image, threshold=0.5):
    return model.tag(pil_image, threshold)
