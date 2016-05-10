#litk

Read/Write 3D images using ITK.

Because Medical Imaging, that's why.

## Dependencies

ITK. Of course.

```bash
    sudo apt-get install libinsighttoolkit4.5 libinsighttoolkit4-dev
```

## Usage

```lua
require 'torch'
require 'litk'

local volume = {
    data = torch.FloatTensor(50, 60, 70),
    origin = torch.FloatTensor {0, 20, 10},
    spacing = torch.FloatTensor {1, 2, 3},
}

volume.data:fill(55)

litk.write3(volume, "volume.mha")
local volume2 = litk.read3("volume.mha")
assert(volume2.data:mean() == 55)
```

