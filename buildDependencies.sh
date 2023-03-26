if [ ! -z "$1" ]; then
    currentDirectory="$1"
fi

if [ -z "$currentDirectory" ]; then
    echo "Missing required environment variable: currentDirectory"
    exit 1
fi

git submodule update --init --recursive

# cd libmba
# mkdir build;cd build
# cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$currentDirectory/install"
# #make install
# #msbuild libmba.sln /t:Build/p:Configuration=Release;Platform=x64  # https://stackoverflow.com/questions/14119498/building-a-solution-file-using-msbuild
# cmake --build . --config Release  # https://stackoverflow.com/questions/8558703/building-msvc-project-with-cmake-and-command-line



cd libmba-0.8.10
mkdir build;cd build
cmake .. -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$currentDirectory/install"
#make install
#msbuild libmba.sln /t:Build/p:Configuration=Release;Platform=x64  # https://stackoverflow.com/questions/14119498/building-a-solution-file-using-msbuild
cmake --build . --config Release  # https://stackoverflow.com/questions/8558703/building-msvc-project-with-cmake-and-command-line






# Demo: bash -c "currentDirectory=$(python -c 'import os; print(os.getcwd())') bash buildDependencies.sh"
