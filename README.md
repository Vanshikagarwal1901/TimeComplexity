# ⏱️ Time Complexity Analyzer – C++ Based

A lightweight and beginner-friendly tool that automatically analyzes the **time complexity** of C/C++ code and outputs its **Big-O notation**.

---

## 📌 Motivation

Understanding the time complexity of algorithms is essential for writing scalable and efficient code. However, manually analyzing time complexity can be challenging—especially for beginners. This project aims to simplify that process by providing a **C++-based static analysis tool** that evaluates and reports time complexity without relying on any external libraries or advanced theoretical knowledge.

---

## 🚀 Features

- 🔍 Parses C/C++ code to detect:
  - Looping structures (`for`, `while`)
  - Conditional statements (`if`, `else`)
  - Simple recursion
- 📈 Estimates time complexity (e.g., `O(1)`, `O(n)`, `O(n²)`)
- 🧠 Supports nested loops and recursive calls
- 💻 Console-based output with optional web frontend (planned)

---

## 🧱 System Architecture (High-Level Flow)

```text
User Code Input
       ↓
   ┌─────────────┐
   │   Parser    │
   └─────────────┘
       ↓
   ┌────────────────────┐
   │ Complexity Analyzer │
   └────────────────────┘
       ↓
   ┌──────────────┐
   │  Output View │
   └──────────────┘
