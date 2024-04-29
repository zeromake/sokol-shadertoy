# sokol-shadertoy

一个使用 sokol 做的 shadertoy 模拟器，可以直接运行 shadertoy 网站上的 glsl 代码。

## 运行方式

### 环境

- OSX: Xcode + command line tools
- Linux: gcc/clang
- Windows: Visual Studio/Mingw

### 下载 shadertoy glsl

> 可以替换 cs2yzG 为自己想要的 shadertoy

``` bash
pwsh ./fetch.ps1 cs2yzG
```

### 如何编译

``` bash
$ xmake f -c -y
$ xmake b -r shader && xmake b shadertoy && xmake r shadertoy
```

## 待办事项

- [x] sokol-shdc 通过 xrepo 自动下载
- [x] sokol 更新到新版的 sg_begin_pass
- [ ] 支持 gpu 的 pporf ui 适配
- [ ] 支持 imgui 的调试 ui 接入
- [ ] 支持 shadertoy 的纹理资源导入
