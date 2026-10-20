"""Locate the MSVC vcvars64.bat used by the desktop/host builds.

Visual Studio is not always installed under %ProgramFiles%\\Microsoft Visual Studio,
so probe the usual environment variables, vswhere and both Program Files roots.
"""
from pathlib import Path
import os
import subprocess


def _candidates():
    for key in ('VCINSTALLDIR', 'VSINSTALLDIR'):
        value = os.environ.get(key)
        if value:
            yield Path(value) / 'VC/Auxiliary/Build/vcvars64.bat'
    for programs in ('ProgramFiles', 'ProgramFiles(x86)'):
        base = os.environ.get(programs)
        if base:
            for path in sorted((Path(base) / 'Microsoft Visual Studio').glob('*/*/VC/Auxiliary/Build/vcvars64.bat')):
                yield path
    vswhere = Path(os.environ.get('ProgramFiles(x86)', r'C:\Program Files (x86)')) / 'Microsoft Visual Studio/Installer/vswhere.exe'
    if vswhere.exists():
        try:
            result = subprocess.run([str(vswhere), '-latest', '-products', '*', '-property', 'installationPath'],
                                    capture_output=True, text=True, timeout=20)
            if result.returncode == 0:
                for line in result.stdout.splitlines():
                    line = line.strip()
                    if line:
                        yield Path(line) / 'VC/Auxiliary/Build/vcvars64.bat'
        except (OSError, subprocess.SubprocessError):
            pass


def find_vcvars():
    for path in _candidates():
        if path.exists():
            return path
    return None
