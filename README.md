## OpengGL  Real-time Renderer
[B站视频](https://www.bilibili.com/video/BV1MyY4eeEVc/)

### Achieved features

1. Use assimp to load model with textures.
2. Free & 360° camera.
3. PBR shdaer with PBR textures set.
4. Realtime Shadow Mapping with PCF, PCSS(VSSM), as well as Compute shaders.
5. Multiple lights, dynamic lights
6. Control panels, Debug panels, Objects management.
7. Skybox.
8. Postprocessing: Gamma correction, HDR.

### Screenshots of Some features

#### Shadows


| Type   | view1                                             | view2                                             |
| ------ | ------------------------------------------------- | ------------------------------------------------- |
| None   | ![1723109802273](images/README/1723109802273.png) | ![1723109811046](images/README/1723109811046.png) |
| Basic  | ![1723109939299](images/README/1723109939299.png) | ![1723109954075](images/README/1723109954075.png) |
| PCF K3 | ![1723110111374](images/README/1723110111374.png) | ![1723110121551](images/README/1723110121551.png) |
| PCF K7 | ![1723110135846](images/README/1723110135846.png) | ![1723110143534](images/README/1723110143534.png) |
| VSSM   | ![1723110167877](images/README/1723110167877.png) | ![1723110174454](images/README/1723110174454.png) |

#### PBR shaders


| Full Term     | ![1723109802273](images/README/1723110510582.png) |
| ------------- | ------------------------------------------------- |
| Specular Term | ![1723110624322](images/README/1723110624322.png) |
| Diffuse Term  | ![1723110631157](images/README/1723110631157.png) |

#### Other features


| Skybox             | ![1723111191865](images/README/1723111191865.png) |
| ------------------ | ------------------------------------------------- |
| Normal mode        | ![1723111201102](images/README/1723111201102.png) |
| Object Manage      | ![1723111208323](images/README/1723111208323.png) |
| Visible Shadow Map | ![1723111286286](images/README/1723111286286.png) |

Reference: [learnopengl.com](learnopengl.com), GAMES 202
