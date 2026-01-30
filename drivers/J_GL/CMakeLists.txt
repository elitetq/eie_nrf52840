zephyr_library()
zephyr_library_sources_ifdef(CONFIG_GPIO j_gl.c)