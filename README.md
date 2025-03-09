```
meson setup build
meson test --print-errorlogs -C build
```

```
meson setup build
meson compile -C build
./build/examples/traffic_lights
```

Requires at least c++17.

Based on [this](https://sii.pl/blog/en/implementing-a-state-machine-in-c17/) article by Michael Adamczyk.