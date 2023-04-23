SETTINGS = {
    'scheduler': 'PNDMScheduler',
    'nsfw_filter': True,
    'use_float16': True,
    'device': 'cuda',
    'gfpgan.arch': 'clean',
    'gfpgan.channel_multiplier': 2,
    'gfpgan.has_aligned': False,
    'gfpgan.only_center_face': False,
    'gfpgan.paste_back': True,
    'gfpgan.weight': 0.5
}

USER_SETTINGS = {
}

def get_setting(name: str, default = '') -> str:
    return USER_SETTINGS.get(
        name, 
        SETTINGS.get(name, default)
    )


def set_user_settings(config: dict):
    print("setting stable diffusion configurations")
    for k in config.keys():
        USER_SETTINGS[k] = config[k]
        # print(f"[settings] {k}: {config[k]}")
