# MeshEditor (C++ STL Processing Tool)

## 📌 Overview

MeshEditor is a C++ command-line application that processes ASCII STL files. It supports generating and manipulating 3D meshes using a modular command-based architecture.

This project is based on a lab assignment focused on building core geometry processing features from scratch, including parsing STL files and generating tessellated meshes.

---

## 🎯 Features

The application supports the following commands:

### 1. Cube

Generates a tessellated cube and writes it to an ASCII STL file.

**Syntax:**

```
Cube L = <double>, origin = (<double>,<double>,<double>), filepath = "<path>"
```

**Example:**

```
Cube L = 10.0, origin = (4.5,3.4,2.1), filepath = "D:\cube.stl"
```

---

### 2. Sphere

Generates a tessellated sphere and writes it to an ASCII STL file.

**Syntax:**

```
Sphere R = <double>, origin = (<double>,<double>,<double>), slices = <uint>, rings = <uint>, filepath = "<path>"
```

**Example:**

```
Sphere R = 10.0, origin = (0,0,0), slices = 16, rings = 16, filepath = "D:\sphere.stl"
```

---

### 3. Split

Splits an input STL mesh into two parts using a plane.

**Syntax:**

```
Split input = "<path>", origin = (<double>,<double>,<double>), direction = (<double>,<double>,<double>), output1 = "<path>", output2 = "<path>"
```

**Example:**

```
Split input = "D:\input.stl", origin = (0,0,0), direction = (0,0,1), output1 = "D:\A.stl", output2 = "D:\B.stl"
```

---

## 🏗️ Project Structure

```
MeshEditor/
│
├── MeshEditor.sln
├── MeshEditor/
│   ├── main.cpp
│   ├── Application.h / Application.cpp
│   ├── Command.h
│   ├── STLParser.h / STLParser.cpp
│   ├── Cube.h / Cube.cpp
│   ├── Sphere.h / Sphere.cpp
│   ├── Split.h / Split.cpp
```

---

## ⚙️ Architecture

### 🔹 Command Pattern

Each operation (Cube, Sphere, Split) is implemented as a command derived from a base `Command` interface.

```cpp
class Command {
public:
    virtual const std::string& getName() const = 0;
    virtual int execute(const std::map<std::string, std::string>& args) = 0;
};
```

### 🔹 Application Core

The `Application` class:

* Registers commands
* Parses CLI input
* Dispatches execution to the appropriate command

### 🔹 STLParser

Handles:

* Reading ASCII STL files
* Writing tessellated meshes
* Computing normals for triangles

---

## 🚀 Getting Started

### Prerequisites

* C++17 compatible compiler
* Microsoft Visual Studio (recommended)

---

### Build & Run

1. Clone the repository:

```
git clone <your-repo-url>
```

2. Open `MeshEditor.sln` in Visual Studio

3. Build the solution:

```
Ctrl + Shift + B
```

4. Run from command line:

```
MeshEditor.exe <command>
```

---

## 🧪 Example Usage

```
MeshEditor.exe Cube L = 5.0, origin = (0,0,0), filepath = "cube.stl"
```

---

## 📐 Key Concepts Covered

* STL file format (ASCII)
* Triangle mesh representation
* Vector math (dot product, cross product, normals)
* Command pattern design
* File I/O in C++

---

## ⚠️ Error Handling

Each command returns specific error codes:

| Code | Meaning                                                     |
| ---- | ----------------------------------------------------------- |
| 0    | Success                                                     |
| 1    | Invalid geometric parameter                                 |
| 2    | Invalid file path                                           |
| 3    | Missing arguments                                           |
| 4    | Operation-specific failure (e.g., no intersection in Split) |

