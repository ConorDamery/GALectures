# **GA Sandbox** 🛠️  

## **Introduction**  
**GA Sandbox** is a hobby project designed for **game developers** who want to explore **Geometric Algebra (GA)** in an environment that feels familiar to game development.  

The application supports **scripting via Wren**, providing a simple and efficient way to interact with the engine.  

### **Recommended Resources for Learning GA**  
- [bivector.net](https://bivector.net/) — Introductory site for geometric algebra concepts  
- [PGA for Computer Science](https://bivector.net/PGA4CS.html) — Projective Geometric Algebra guide  
- [Dual Quaternions Demystified (and more!)](https://www.youtube.com/watch?v=ichOiuBoBoQ) — GAME2020 talk by Steven De Keninck
- [Look Ma, No Matrices!](https://enkimute.github.io/LookMaNoMatrices/) — Visual intro to transformations with PGA  
- [ganja.js](https://enkimute.github.io/ganja.js/) — JavaScript library for geometric algebra experiments  
- [bivector.net Discord](https://discord.com/invite/vGY6pPk) — Join the community to discuss Geometric Algebra

## **Getting Started**  
- **Scripting**: The entire engine is exposed to Wren through a single class called `App`.  
- **Documentation**: The `App` class contains **well-documented functions** with explicit type information, ensuring clarity on what the native code expects.  
- **Wren Docs**: For more information about Wren, visit their official documentation: [https://wren.io/](https://wren.io/)  

## **Creating a Scene**  
To create and display a custom scene in **GA Sandbox**, follow these steps:  

1. **Create a script file:**  
   - Write a `main.wren` script.  
   - This will be the entry point for your scene logic.

2. **Register your script:**  
   - Add the path to your `main.wren` file into the `index.txt` file.  
   - The first item listed will be the initial scene loaded by the application.  
   - You can dynamically load other scenes from Wren at runtime.

3. **About `main.wren` files:**  
   - Multiple `main.wren` files can exist in the `Assets` folder.
   - Only those listed in `index.txt` will be recognized and used.
   - Any `main.wren` file is **excluded from the precompilation step**, allowing it to define scene entry points.

4. **Typical `main.wren` structure:**

```wren
import "app" for App // Get the app class to use native callbacks

// Useful class to avoid static access all the time in Main
class State {
    // Initialize everything here
    construct new() {
        // Load assets, create variables, etc.
    }

    // Called every frame unless app is paused
    update(dt) {
        // Update variables like camera or object positions!
    }

    // Called every frame, use for gui and graphics
    render() {
        App.glClear(0.1, 0.1, 0.1, 1, 0, 0, 0)
        // Set shader, etc. And draw some stuff!
    }
}

class Main {
    static init() {
        __state = State.new()
    }
    static update(dt) {
        __state.update(dt)
    }
    static render() {
        __state.render()
    }
}
```

## **Managing Assets**  
- The **manifest file** should list all files inside the `Assets/` folder.  
- Any file **not listed** in the manifest will **not be detected** by the application.  
- All `.wren` files (except `main.wren`) will be **precompiled at startup** or when the app is reloaded.  
- No two scenes will exist at the same time.  

## **Have Fun!** 🎮  
Enjoy exploring Geometric Algebra in **GA Sandbox** and feel free to experiment with new scenes and scripts!