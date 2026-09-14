# nRF7002 DK: Build Flashes Successfully but the Application Does Not Run

## Symptoms

- The target is `nrf7002dk/nrf5340/cpuapp/ns`.
- Build and flash both report success.
- The LED does not blink and the serial console shows no output.
- Pressing the reset button does not help.

## Root Cause

The `/ns` target uses a multi-image boot layout:

| Flash address | Image |
| --- | --- |
| `0x00000000` | MCUboot bootloader |
| `0x00010000` | Trusted Firmware-M (TF-M) |
| `0x00050000` | Non-secure Zephyr application |

The build reserved address `0x00000000` for MCUboot, but MCUboot was not enabled. The nRF5340 therefore started an old or invalid image at the reset address and never reached `main()`. A normal flash operation did not fix this because it erased only the address ranges used by TF-M and the application.

## Fix

### 1. Enable MCUboot

Create `sysbuild.conf` in the application root:

```conf
SB_CONFIG_BOOTLOADER_MCUBOOT=y
```

Full path:

```text
D:\ESE518_Projct\Blinky\sysbuild.conf
```

### 2. Configure MCUboot for the nRF7002 DK

Create `sysbuild/mcuboot.conf`:

```conf
CONFIG_SPI=n
CONFIG_SPI_NOR=n
```

Full path:

```text
D:\ESE518_Projct\Blinky\sysbuild\mcuboot.conf
```

These settings keep this simple application on internal flash and disable unused SPI NOR support in MCUboot.

### 3. Perform a pristine sysbuild

Run this in an **nRF Connect terminal**:

```powershell
west build -p always -d D:\ESE518_Projct\Blinky\build -b nrf7002dk/nrf5340/cpuapp/ns --sysbuild D:\ESE518_Projct\Blinky
```

The build output should show that the `mcuboot`, `tfm`, and application images were created.

### 4. Erase the stale image and flash the complete build

```powershell
west flash -d D:\ESE518_Projct\Blinky\build --dev-id 1050760700 --erase
```

Flash the top-level build. Do not add `--domain Blinky`, because the board needs both MCUboot and the application images. The `--erase` option removes the old image that was still located at the reset address.

## Why This Works

MCUboot is now programmed at `0x00000000`, where the nRF5340 begins execution after reset. MCUboot then validates and starts the signed TF-M and Zephyr application images at their configured addresses. As a result, execution reaches `main()`, so GPIO control and serial output work normally.

This was a boot-chain configuration problem, not an LED, GPIO, overlay, or hardware failure.
