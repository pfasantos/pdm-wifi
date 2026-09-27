# pdm-wifi

ESP-IDF firmware for an ESP32-S3 that reads microphone data through I2S, converts it to 16-bit samples, and streams raw buffers to a host over TCP. The firmware uses the two-stage CIC and post-filter implementation supplied by `pdm2pcm.c`; those files are maintained separately and are not modified here.

## Requirements and wiring

- ESP-IDF with `idf.py` and its `protocol_examples_common` example component available.
- An ESP32-S3 target, Python 3, and a host that can receive a TCP byte stream.
- The project code assigns I2S BCLK to GPIO 5 and data input to GPIO 4. MCLK, WS, and data output are disabled. Connect microphone power, ground, and select pins according to the board and microphone documentation.

## Configure and run

1. In an ESP-IDF shell, run `idf.py set-target esp32s3` for a new build configuration.
2. Run `idf.py menuconfig`. Set the Wi-Fi connection through the imported example connection component. Under **PDM recorder configuration**, set the TCP server IPv4 address and port, recording duration, I2S startup and recording rates, and DMA buffer sizing as needed.
3. Set `TCP_IP`, `TCP_PORT`, `CHUNK_SIZE`, and `OUTPUT_DIR` in `tcp_receiver.py` to match the board and capture settings, then start the receiver with `python3 tcp_receiver.py`. The default chunk size is 4,088 bytes and the default port is 8888. The receiver saves raw bytes without a WAV header.
4. Build, flash, and monitor:

   ```sh
   idf.py build
   idf.py -p PORT flash monitor
   ```

Replace `PORT` with the board's serial device. The defaults preserve the current firmware settings: the existing server address and port defaults, 8 kHz startup rate, 75 kHz recording rate, 8 DMA buffers with 511 frames each, and a two-minute recording timer.

## Data path and limits

`app_main()` initializes I2S and Wi-Fi, creates a reader task, a TCP sender task, a queue, and a one-shot recording timer. The reader converts each buffer to 16-bit samples and queues it. The sender writes buffers with `send()` over a TCP stream. TCP preserves byte order while the connection remains usable, but individual `send()` calls do not define message boundaries. The current sender does not retry a short write, so the receiver and firmware depend on complete 4,088-byte buffer writes for alignment. The data has no packet headers, timestamps, or sequence numbers.

The default configured I2S clock changes from 8,000 Hz during initialization to 75,000 Hz before capture. The effective sample rate and output channel interpretation should be checked with the actual board and signal. The queue can fill if the sender cannot keep up, and the firmware does not record timestamps or queue-loss counters.

The source configuration selects the ESP32-S3 I2S pins listed above. Wi-Fi credentials and other generated build settings are supplied locally by ESP-IDF.

## Code style

C sources use the checked-in `.clang-format` settings and FreeRTOS-style type prefixes for functions and variables. Keep application constants in uppercase, module-prefixed macros; place values that need routine tuning in the **PDM recorder configuration** menu. Format with `clang-format -i main/main.c main/main.h main/i2s_std.c main/i2s_std.h`. `pdm2pcm.c` and `pdm2pcm.h` are intentionally excluded.

## API documentation

Generate the Doxygen HTML reference with `doxygen Doxyfile` from the repository root, then open `build/html/index.html`. Generated files stay under the ignored `build/` directory.
