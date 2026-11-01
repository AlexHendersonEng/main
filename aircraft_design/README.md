# Aircraft Design

## OpenVSP Configuration

```PowerShell
# Set the OpenVSP installation directory
$env:OPENVSP_ROOT = "C:\path\to\OpenVSP-3.55.0-win64-Python3.13\OpenVSP-3.55.0-win64"

# Create a directory junction in the project root
New-Item -ItemType Junction -Path OpenVSP -Target $env:OPENVSP_ROOT

# Install project dependencies
uv sync
```

```bash
# Set the OpenVSP installation directory
export OPENVSP_ROOT="/path/to/OpenVSP"

# Create a symbolic link in the project root
ln -s "$OPENVSP_ROOT" OpenVSP

# Install project dependencies
uv sync
```
