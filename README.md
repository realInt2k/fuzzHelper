# fuzzHelper
very nice helper using clang 7 and sanitizer + compiler rt

go to the SVF folder, and like "source build.sh", read the log, if something isn't found then you probably have to install library for it otherwise it'll not be built.
then go to the llvm7 folder and like mkdir build/, then copy the install sh thing to that build, then run that install sh thing. Then, type ninja to build the llvm+clang

to run the pass, you'll use -fsanitize="handler8Helper" (i think)
