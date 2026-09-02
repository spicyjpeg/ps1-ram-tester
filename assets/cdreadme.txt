
                               - ps1-ram-tester -
                                 Version ${PROJECT_VERSION}

[ Usage ]

Burn the provided disc image and boot it as usual (directly from the BIOS on a
modchipped console or through a loader such as Unirom or tonyhax). Most of the
available options should be self-explanatory.

For more information and source code, see:
    <${PROJECT_HOMEPAGE_URL}>

NOTE: the CD-ROM image contains license data suitable for Japanese region
consoles. It is compatible with all models and regions with the exception of the
PAL PSone, for which one of the following workarounds is required:

- installing a modchip with BIOS "region patching" functionality;
- patching the disc image with PAL license data prior to burning;
- editing the license region string in cdrom.json and recompiling the tester;
- launching via an intermediate loader that skips the license data checks in the
  BIOS shell;
- launching through disc swapping (not recommended).

[ License ]

Copyright (C) 2026 spicyjpeg

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
