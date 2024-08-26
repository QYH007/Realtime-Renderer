## OpengGL  Real-time Renderer

[B站视频](https://www.bilibili.com/video/BV1MyY4eeEVc/)(虽然这个视频需要更新，里面有些新功能没录上)

### Achieved features

1. Realtime Shadow Mapping with PCF, PCSS(VSSM)
2. Fixed position point Environment mapping with IBL
3. Deferred Rendering Pipeline (include SSAO)
4. PBR shading with PBR textures set.
5. NPR shading
   1. outline
   2. shading (developing)
6. Use assimp to load model with textures.
7. Multiple lights, dynamic lights
8. Control panels, Debug panels, Objects management.
9. Skybox.
10. Postprocessing: Gamma correction, HDR.
11. Free & 360° camera

### Screenshots of Some features

#### 1. Shadows


| Type   | view1                                             | view2                                             |
| ------ | ------------------------------------------------- | ------------------------------------------------- |
| None   | ![1723109802273](images/README/1723109802273.png) | ![1723109811046](images/README/1723109811046.png) |
| Basic  | ![1723109939299](images/README/1723109939299.png) | ![1723109954075](images/README/1723109954075.png) |
| PCF K3 | ![1723110111374](images/README/1723110111374.png) | ![1723110121551](images/README/1723110121551.png) |
| PCF K7 | ![1723110135846](images/README/1723110135846.png) | ![1723110143534](images/README/1723110143534.png) |
| VSSM   | ![1723110167877](images/README/1723110167877.png) | ![1723110174454](images/README/1723110174454.png) |

#### 2. Environment Mapping(fixed point)

Scene with IBL
![1724507349707](images/README/1724507349707.png)

Scene with defualt ambient color
![1724507757168](images/README/1724507757168.png)


| Env Only      | ![1724507481144](images/README/1724507481144.png) |
| ------------- | ------------------------------------------------- |
| Light Only    | ![1724507521229](images/README/1724507521229.png) |
| Mirror like 1 | ![1724507784127](images/README/1724507784127.png) |
| Mirror like 2 | ![1724507829903](images/README/1724507829903.png) |

#### 3. Deferred Rendering

Overveiw


| ![1723797046707](images/README/1723797046707.png) |
| ------------------------------------------------- |
| ![1723796594646](images/README/1723796594646.png) |

G-buffers


| ![300](images/README/1723795608339.png)position              | ![1723795751750](images/README/1723795751750.png)  depth    |
| ------------------------------------------------------------ | ----------------------------------------------------------- |
| ![1723795772408](images/README/1723795772408.png)  normal    | ![1723795776483](images/README/1723795776483.png)  color    |
| ![1723795809676](images/README/1723795809676.png)  metalness | ![1723795821766](images/README/1723795821766.png) roughness |

SSAO-Furina


|            | ssaoRadius:1                                      | ssaoRadius:3                                      |
| ---------- | ------------------------------------------------- | ------------------------------------------------- |
| BlurSize:1 | ![1723796427442](images/README/1723796427442.png) | ![1723796460906](images/README/1723796460906.png) |
| BlurSize:4 | ![1723796442166](images/README/1723796442166.png) | ![1723796468969](images/README/1723796468969.png) |

SSAO-Gun


| ![1723796695814](images/README/1723796695814.png) |
| ------------------------------------------------- |
| ![1723796702561](images/README/1723796702561.png) |

#### 4. NPR shading (outline only)

![1724659915892](images/README/1724659915892.png)

![1724660059550](images/README/1724660059550.png)

![1724661167737](images/README/1724661167737.png)

![1724660069619](images/README/1724660069619.png)

![1724660982404](images/README/1724660982404.png)

![1724660988384](images/README/1724660988384.png)

#### 5. PBR shaders


| Full Term     | ![1723109802273](images/README/1723110510582.png) |
| ------------- | ------------------------------------------------- |
| Specular Term | ![1723110624322](images/README/1723110624322.png) |
| Diffuse Term  | ![1723110631157](images/README/1723110631157.png) |

#### 6. Other features


| Skybox             | ![1723111191865](images/README/1723111191865.png) |
| ------------------ | ------------------------------------------------- |
| Normal mode        | ![1723111201102](images/README/1723111201102.png) |
| Object Manage      | ![1723111208323](images/README/1723111208323.png) |
| Visible Shadow Map | ![1723111286286](images/README/1723111286286.png) |

Reference: [learnopengl.com](learnopengl.com), GAMES 202
