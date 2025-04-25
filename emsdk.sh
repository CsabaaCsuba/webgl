sudo apt-get update
sudo apt-get install cmake python3 nodejs npm git
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest

echo 'export PATH="/home/csabi/Desktop/webgl/emsdk:$PATH"' >> ~/.bashrc
echo 'export PATH="/home/csabi/Desktop/webgl/emsdk/upstream/emscripten:$PATH"' >> ~/.bashrc
echo 'source "/home/csabi/Desktop/webgl/emsdk/emsdk_env.sh"' >> $HOME/.bash_profile
source emsdk_env.sh