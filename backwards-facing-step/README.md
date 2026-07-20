# Backwards Facing Step

## Overview

This directory contains an OpenFOAM case for the classic backwards-facing-step (BFS) benchmark. The case demonstrates separated flow, recirculation behind a step and can be run in serial or parallel.

## Prerequisites

- OpenFOAM installed and available on your PATH.
- ParaView or another VTK/VTU-capable viewer for post-processing.

## Quick start

Run the case using the provided scripts:

Serial run:

```bash
cd case
./Allrun
```

Parallel run:

```bash
cd case
./Allrun-parallel
```

## License

This case follows the repository license: [LICENSE](../LICENSE)
