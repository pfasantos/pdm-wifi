# pdm-wifi

ESP-IDF firmware for an ESP32-S3 that acquires microphone data with I2S, converts it with a two-stage CIC and a short post-filter, and sends raw 16-bit sample buffers to a host over UDP. A Python receiver in this repository saves the datagrams to a file. This is the wireless counterpart to [pdm32bits-rtos](https://github.com/pfasantos/pdm32bits-rtos).

The project experiments described in the author's final IC report used an SPH0641LU4H-1 PDM microphone and an ESP32-S3. The source configures I2S standard-mode RX with 32-bit stereo slots; it does not use the ESP32-S3 hardware PDM-to-PCM converter.

## Requirements and wiring

- ESP-IDF with `idf.py` and its `protocol_examples_common` example component available. The project CMake file imports that component from `$IDF_PATH/examples/common_components/`.
- An ESP32-S3. `dependencies.lock` records `target: esp32s3`.
- A host with Python 3 and UDP connectivity to the board.

| Function | ESP32-S3 GPIO | Source |
| --- | ---: | --- |
| I2S bit clock | 5 | `main/i2s_std.c` |
| I2S data input | 4 | `main/i2s_std.c` |

MCLK, WS and I2S data output are disabled in the source. The microphone power, ground and any select pins must be wired according to the actual board and microphone datasheet; those connections are not specified by this repository.

## Configure and run

1. In an ESP-IDF shell, run `idf.py set-target esp32s3` if you are creating a new configuration.
2. Run `idf.py menuconfig` and configure the Wi-Fi connection through the imported example connection component.
3. Set the receiver's IPv4 address and UDP port in `main/main.h` (`SERVER_IP_ADDR` and `SERVER_PORT`). They are currently hardcoded as `10.0.0.48:8888`. `main/Kconfig.projbuild` also declares server settings, but `main.c` uses the header macros, so changing only the menu values will not change the destination.
4. On the host, set `UDP_PORT` and `OUTPUT_FILE` in `udp_receiver.py`, then start the receiver:

   ```sh
   python3 udp_receiver.py
   ```

5. Build, flash and monitor the board:

   ```sh
   idf.py build
   idf.py -p PORT flash monitor
   ```

Replace `PORT` with the board's serial device. Start the receiver before starting capture. The receiver writes to `resultados/teste_5khz` by default and exits after 15 seconds without a packet or on Ctrl+C.

## Data path and format

`app_main()` initializes I2S and Wi-Fi, then creates a reader task, UDP sender task, queue and one-shot recording timer. The reader starts the timer, reads 4,088-byte I2S buffers, converts them to 16-bit sample buffers, and queues them. The sender sends each buffer as one 4,088-byte UDP datagram. The default timer interval is `REC_TIME_MS` (two minutes). The receiver concatenates datagram payloads into a raw file without a WAV header, packet metadata or sequence numbers.

The I2S configuration starts at 8,000 and is reconfigured to 75,000 before capture. The final IC report describes experiments at a nominal 75 kHz sampling rate and 4.8 MHz microphone clock. Those are experiment settings; the effective sample rate and output channel interpretation should be verified with the actual board and captured signal.

## Limits to account for

- UDP gives no delivery or order guarantee. The current datagrams are larger than a typical network MTU and may be fragmented. The receiver cannot detect missing, duplicate or reordered datagrams because packets have no sequence numbers.
- The receiver's printed expected packet count is a fixed 743-packet estimate. It does not reflect the two-minute timer or actual runtime, so its loss percentage is only a rough diagnostic.
- If the sender cannot keep up, the queue can fill and block the reader. The firmware does not record per-packet timestamps or queue-loss counters.
- When the reader stops, it notifies the sender, which closes and reopens its socket; the code does not explicitly drain the queue or terminate the sender task.
- The project is configured for the ESP32-S3 in the lockfile, but no `sdkconfig` is committed. Wi-Fi credentials and other build settings must be supplied locally.

## API documentation

Generate the Doxygen HTML reference with `doxygen Doxyfile` from the repository root, then open `build/doxygen/html/index.html`. Generated files stay under the ignored `build/` directory.
