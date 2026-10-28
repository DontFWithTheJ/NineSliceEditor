[**English**](README.md) · [Русский](README.ru.md)

# NineSliceEditor

Edit and preview 9-slice margins for Slate Brushes directly on the texture in Unreal Engine.

NineSliceEditor adds an **Edit** button next to the `Margin` property of a Slate Brush. Clicking it opens a visual editor where all four margins can be adjusted directly on the texture.

**Current release:** v1.0.0 · **Unreal Engine:** 5.7

![NineSliceEditor interface](images/editor_interface.png)

## Contents

- [What Is 9-Slice?](#what-is-9-slice)
- [Using the Plugin](#using-the-plugin)
  - [Opening the Editor](#opening-the-editor)
  - [Editing Margins](#editing-margins)
  - [Mouse Controls](#mouse-controls)
  - [Zoom Controls](#zoom-controls)
  - [Color Controls](#color-controls)
  - [Preview](#preview)
  - [Apply and Cancel](#apply-and-cancel)
- [Installation](#installation)
  - [Installing from a Release](#installing-from-a-release)
  - [Enabling the Plugin](#enabling-the-plugin)
  - [Installing for an Engine Version](#installing-for-an-engine-version)
  - [Getting the Source Code](#getting-the-source-code)
- [Building from Source](#building-from-source)
  - [Packaging with Unreal Automation Tool](#packaging-with-unreal-automation-tool)
  - [Building Through an Unreal Project](#building-through-an-unreal-project)
- [Releases](#releases)
- [License](#license)

---

## What Is 9-Slice?

9-slice scaling is a way to resize a texture without distorting its corners. It is commonly used for UI elements such as buttons, panels, windows, and borders, which need to fit different sizes while keeping their original appearance.

Without 9-slice margins, the entire texture stretches when the widget changes size. This can make corners look wider or flatter than intended.

![Texture stretched without 9-slice margins](images/9slice_no_margins.png)

With 9-slice scaling, four margins divide the texture into nine areas:

- **Four corners** keep their original size.
- **Four edges** stretch in one direction.
- **The center** stretches in both directions.

The margins determine where these areas begin and end. The image below shows how the texture is divided.

![Texture divided by 9-slice margins](images/9slice_margins.png)

When the brush is resized, the edges and center adapt to the new dimensions while the corners retain their original size.

![Result of 9-slice scaling](images/9slice_result.png)

In Unreal Engine, these margins are set on the Slate Brush. Adjusting them by entering values in the Details panel can be inconvenient, especially when the correct boundaries are not obvious from the numbers alone.

NineSliceEditor makes this easier by displaying the texture and its margins in a visual editor. Move the guides directly on the image and check the result in the preview.

## Using the Plugin

### Opening the Editor

1. Select an asset or widget that exposes a Slate Brush in the **Details** panel.
2. Find the **Margin** property.
3. Make sure **Draw As** is set to **Box** or **Border**.
4. Click **Edit** next to **Margin**.

![Edit button next to the Margin property](images/details_edit_button.png)

The NineSliceEditor window will open with the brush texture and its current margins.

![NineSliceEditor window](images/editor_interface.png)

The Margin property is used for the **Box** and **Border** draw modes. Other draw modes do not use these margins in the same way.

### Editing Margins

The editor displays four guides over the texture:

- **Left**
- **Top**
- **Right**
- **Bottom**

Drag a guide with the left mouse button to change the corresponding margin. The guides show the boundaries between the fixed corners, the stretching edges, and the center.

Margin values can be displayed in three units:

- **Pixels** — values based on the texture's pixel dimensions.
- **Normalized** — values expressed relative to the texture dimensions.
- **Percentage** — values expressed as percentages.

Changing the display unit changes how the values are shown, not the position of the margins.

### Mouse Controls

| Input | Action |
| --- | --- |
| LMB + drag a guide | Change the selected margin |
| RMB + drag | Move the texture in the editor |
| Mouse wheel | Zoom in or out |

### Zoom Controls

The editor also includes buttons for controlling the image view:

- **−** — zoom out.
- **+** — zoom in.
- **Fit** — fit the entire texture into the available space.

These controls change the view, not the margin values.

### Color Controls

The editor lets you change the colors of three visual elements:

- **Background** — the editor window background.
- **Guide** — the margin guides and their handles.
- **Canvas** — the background behind the texture in the editing area.

Presets are available for each category. These settings help keep the guides visible against textures with different colors and levels of transparency.

### Preview

The **Preview** panel shows how the current margins behave when the brush is resized.

It contains three examples:

- **Horizontal** — the brush is resized horizontally.
- **Vertical** — the brush is resized vertically.
- **Both** — the brush is resized in both directions.

The preview also has scale controls so you can inspect the result at different sizes.

Use the preview to check that the corners and edges behave as expected before applying the changes.

### Apply and Cancel

When you finish editing, choose one of the following:

- **Apply** — saves the current margins to the brush and closes the editor.
- **Cancel** — closes the editor without applying the changes.

---

## Installation

### Installing from a Release

The easiest way to install NineSliceEditor is to download a packaged release.

1. Open the [Releases](../../releases) page.
2. Download the ZIP package for your Unreal Engine version.
3. Close Unreal Engine if it is running.
4. Extract the archive.
5. Copy the `NineSliceEditor` plugin folder into your project's `Plugins` directory. Create the directory if it does not exist.

For example:

```text
YourProject/
└── Plugins/
    └── NineSliceEditor/
        ├── NineSliceEditor.uplugin
        └── ...
```

The exact contents depend on the release package. The important part is that `NineSliceEditor.uplugin` is directly inside the `NineSliceEditor` folder, not inside another folder with the same name.

### Enabling the Plugin

After copying the plugin into your project:

1. Open the project in Unreal Engine.
2. Select **Edit → Plugins** from the top menu.
3. Search for `NineSliceEditor`.
4. Enable the plugin using the checkbox.
5. Restart Unreal Engine if prompted.

![NineSliceEditor enabled in the Plugins window](images/plugin_enabled.png)

If the plugin does not appear in the list, check that the `.uplugin` file is in the correct directory and that you installed a package compatible with your engine version.

### Installing for an Engine Version

You can also install the plugin into the engine's plugin directory. This makes it available to projects using that specific Unreal Engine installation.

For example, for Unreal Engine 5.7 installed through the Epic Games Launcher on Windows:

```text
C:\Program Files\Epic Games\UE_5.7\Engine\Plugins\Marketplace\NineSliceEditor\
```

The location may differ depending on where Unreal Engine is installed.

1. Close Unreal Engine.
2. Copy the packaged `NineSliceEditor` folder into the engine's plugin directory.
3. Make sure `NineSliceEditor.uplugin` is directly inside the `NineSliceEditor` folder.
4. Launch your project and enable NineSliceEditor through **Edit → Plugins** if necessary.

Use a package built for the same Unreal Engine version as the installation. If you only have the source code or need a package for another engine version, follow the instructions in [Building from Source](#building-from-source).

This installation applies to one engine installation. It does not automatically install the plugin for every Unreal Engine version on your computer.

**Which installation should you choose?**

- **Project installation:** use this if you only need NineSliceEditor in a particular project.
- **Engine installation:** use this if you want the plugin to be available to multiple projects using the same engine installation.

### Getting the Source Code

To get the plugin source code, clone the repository:

```bash
git clone https://github.com/DontFWithTheJ/NineSliceEditor.git
```

Alternatively, open the repository on GitHub, select **Code → Download ZIP**, and extract the archive.

The repository contains the plugin source code. If you want to use it in a project, modify it, or build it for another Unreal Engine version, follow the appropriate build instructions below.

## Building from Source

Building from source is only necessary if you want to modify the plugin, debug it, or prepare a package for an Unreal Engine version that does not have a suitable release.

There are two approaches, depending on how you intend to use the plugin.

### Packaging with Unreal Automation Tool

Use this method to package the plugin independently of a project. The resulting package can then be installed into a project or an engine installation.

On Windows, open Command Prompt and run the following command. Replace the example paths with the actual paths on your computer.

```bat
"C:\Path\To\UnrealEngine\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="C:\Path\To\NineSliceEditor\NineSliceEditor.uplugin" -Package="C:\Path\To\Output\NineSliceEditor" -TargetPlatforms=Win64
```

For example, if Unreal Engine 5.7 is installed at `C:\Program Files\Epic Games\UE_5.7`, the path to `RunUAT.bat` will be:

```text
C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\RunUAT.bat
```

The command arguments are:

- `RunUAT.bat` — the Unreal Automation Tool script from the engine version you want to build with.
- `-Plugin` — the path to the source plugin's `NineSliceEditor.uplugin` file.
- `-Package` — the output directory where the packaged plugin will be created.
- `-TargetPlatforms=Win64` — builds the plugin for Windows 64-bit.

The source plugin directory must contain `NineSliceEditor.uplugin`.

When the process finishes, check the output directory specified by `-Package`. It should contain the packaged plugin.

**To install the packaged plugin in a project**, copy its `NineSliceEditor` folder into:

```text
YourProject/Plugins/NineSliceEditor/
```

**To install it for an engine installation**, copy the same folder into:

```text
<UnrealEngine>/Engine/Plugins/Marketplace/NineSliceEditor/
```

In both cases, keep `NineSliceEditor.uplugin` directly inside the `NineSliceEditor` folder. Open the project and enable the plugin through **Edit → Plugins** if necessary.

Use `RunUAT.bat` from the Unreal Engine version you are targeting. Building with one engine version does not automatically make the package compatible with other versions.

### Building Through an Unreal Project

Use this method when you want to work on the plugin's C++ source code from within an Unreal Engine project.

1. Close Unreal Engine.
2. Copy the source `NineSliceEditor` folder into your project's `Plugins` directory.
3. Check that the descriptor is located at `YourProject/Plugins/NineSliceEditor/NineSliceEditor.uplugin`.
4. Generate or refresh the project files if required by your IDE.
5. Open the project in your C++ IDE.
6. Build the project's **Development Editor** target for **Win64**.
7. Open the project and enable NineSliceEditor through **Edit → Plugins** if necessary.

This method is intended for development and debugging. The project must have a working Unreal Engine C++ build environment.

If the plugin does not compile with a different Unreal Engine version, source code changes may be required because engine APIs and build requirements can change between releases.

---

## Releases

| Version | Unreal Engine | Download |
| --- | --- | --- |
| v1.0.0 | UE 5.7 | [GitHub Release](../../releases/tag/v1.0.0) |

## License

NineSliceEditor is free to use and modify, including in commercial projects. Redistribution of the original or modified plugin is permitted only under the terms of the included license.

Selling the plugin itself or offering it as a paid product is prohibited. Projects created using the plugin may be distributed and sold commercially.

See the [LICENSE](LICENSE) file for the full terms.
