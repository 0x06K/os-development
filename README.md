# :)

The script (setup-tools.sh) fetches the latest GCC and Binutils from the GNU FTP, builds them from source, and installs NASM, QEMU, GDB, and Make alongside them. When it's done, you can use the Makefile to compile and run the code. 

<img width="1000" height="534" alt="image" src="https://github.com/user-attachments/assets/63b61f2c-8a0a-483c-bfb4-b6649ab795b9" />

## Usage
<pre><code>chmod +x setup-tools.sh
./setup-tools.sh

# to compile and run the code. i mean just read it already bruh :3
make run
</code></pre>

Note: the image gets mounted at `/mnt/os` to drop `user.bin` into the OS image so `make run` will ask for sudo.

## Shell Commands
You can use these commands in shell.

```
cat   <file>              print file contents
touch <file>              create empty file
rm    <file>              delete file
mv    <src> <dst>         rename or move file
cp    <src> <dst>         copy file
ls                        list current directory
mkdir <dir>               create directory
mkdir -p <path>           create nested directories
rmdir <dir>               remove empty directory
rmdir -r <dir>            remove directory recursively
cd    <path>              change directory
pwd                       print working directory
stat  <path>              file or directory info
find  <name>              search recursively
df                        disk free space
du    <file>              disk usage of a file
sync                      flush disk writes
echo  <text>              print to terminal
echo  <text> > <file>     write text to file
help                      show command list
clear                     clear the screen
version                   show shell version
```

This code may contain some bugs in rmdir command anyway code efficiency was never focus though i was just trying to learn some theoretical concepts by implementing them. Feel free to make changings in the code. I will upload the code documentation in a few days.
