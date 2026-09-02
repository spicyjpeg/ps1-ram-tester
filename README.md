
# PlayStation 1 RAM, VRAM and SPU RAM testing tool

This is a simple but feature-complete memory testing tool for the original Sony
PlayStation, allowing for exhaustive testing of all of the console's RAM chips:

- main RAM testing is fully supported up to 16 MB on all PS1 revisions, with
  proper handling of all configurations permitted by the CPU's DRAM controller
  (single or dual bank, 1/2/4/8 MB per bank). In order to ease testing of
  PS1-based arcade boards and "upgraded" consoles with non-standard main RAM
  geometry, a menu is provided to edit all DRAM controller options, including
  ones whose purpose is currently unknown;
- VRAM testing is supported up to 1 or 2 MB depending on GPU revision. Console
  models equipped with the 208-pin GPU (all of them except for development kits,
  the SCPH-1000 and a limited number of other SCPH-1xxx models) support
  addressing up to two 1 MB banks;
- SPU RAM testing is supported up to 512 KB or 4 MB depending on SPU revision.
  Console models equipped with the standalone 100-pin SPU (all ones prior to
  SCPH-7xxx) may address up to two banks of 2 MB each. As with main RAM, all
  configuration flags can be edited including unknown ones.

This tester can also be used to launch a game or application from the CD-ROM
with customized default main RAM, VRAM and SPU RAM configuration. This is useful
for instance to run ROM hacks or homebrew software that require additional
memory but do not explicitly initialize the DRAM controller.

Finally, this tester serves as a practical example of a simple C application
built on top of the
[ps1-bare-metal](https://github.com/spicyjpeg/ps1-bare-metal) headers and build
system.

## Download and usage

The latest version of the tester can be found on the releases page, accessible
through the GitHub sidebar. Each release is available as a CD-ROM image or as a
standalone PS1 executable file (e.g. for loading through a serial cable).

**NOTE**: the CD-ROM images contain license data suitable for Japanese region
consoles. They are compatible with all models and regions with the exception of
the PAL PSone, for which one of the following workarounds is required:

- installing a modchip with BIOS "region patching" functionality;
- patching the disc image with PAL license data prior to burning;
- editing the license region string in `cdrom.json` and recompiling the tester;
- launching via an intermediate loader that skips the license data checks in the
  BIOS shell such as Unirom or tonyhax;
- launching through disc swapping (not recommended).

## Screenshots

<p align="center">
  <img alt="Main menu" src="doc/assets/mainMenu.png" width="320" />
  <img alt="RAM configuration menu" src="doc/assets/ramConfigMenu.png"
    width="320" />
  <img alt="Test screen" src="doc/assets/testScreen.png" width="320" />
</p>

## Building the tester

This repository follows the same overall structure as ps1-bare-metal, so you may
refer to
[its build instructions](https://github.com/spicyjpeg/ps1-bare-metal#building-the-examples).

If MAME's `chdman` tool is installed and listed in your `PATH` environment
variable, it will be automatically used to generate a CHD version of the CD-ROM
image in addition to the raw `.iso` file. You may also specify its location
manually by passing `-DCHDMAN_PATH` to CMake while configuring the project.

## License

As with ps1-bare-metal, everything in this repository is licensed under the MIT
license (or the functionally equivalent ISC license). The only "hard"
requirements are attribution and preserving the license notice; you may
otherwise freely use any of the code for both non-commercial and commercial
purposes.

## See also

- The
  ["Memory Control" section of psx-spx](https://psx-spx.consoledev.net/memorycontrol)
  has further information on main RAM configuration.
- If you need help or wish to discuss PS1 homebrew development more in general,
  you may want to check out the
  [PSX.Dev Discord server](https://discord.gg/QByKPpH).
