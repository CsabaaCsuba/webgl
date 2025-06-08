import subprocess
from pathlib import Path

SRC_DIR = Path("src")
BUILD_DIR = Path("build")
BUILD_DIR.mkdir(exist_ok=True)

CLANG = "clang"
OUTPUT = BUILD_DIR / "main"

CFLAGS = [
    "-std=c23",
    "-O2",
    "-Wall",
    "-I/usr/include",
    "-I/usr/include/freetype2",
    "-I/usr/include/harfbuzz",
    "-L/usr/lib/x86_64-linux-gnu",
    "-lglfw",
    "-lGLESv2",
    "-lfreetype",
    "-lharfbuzz",
    "-lm",
]

def build():
    src_files = list(SRC_DIR.glob("*.c"))
    if not src_files:
        print("Error: No source files found in", SRC_DIR)
        return

    cmd = [CLANG] + CFLAGS + [str(src) for src in src_files] + ["-o", str(OUTPUT)]
    print("Compiling and linking:", " ".join(cmd))
    
    try:
        subprocess.run(cmd, check=True)
        print(f"✅ Build complete: {OUTPUT}")
        return True
    except subprocess.CalledProcessError as e:
        print(f"❌ Build failed with error code {e.returncode}")
        return False

if __name__ == "__main__":
    if build():
        subprocess.run([str(OUTPUT)])