
## ⚠️ Before You Start
Make sure you already have the following installed:

* **GCC compiler**
* **Linux environment** (Ubuntu recommended)
* **nano editor**
* **Network access**

**Optional tools:**
* Wireshark (for packet analysis)
* Netcat (`nc`)
* Telnet

---

## 📝 Step 1: Create the Source File
Open your terminal and create the C source file using the `nano` editor:

```bash
nano client.c
nano serveur.c
```


## ⚙️ Step 2: Compile the Code

Once your code is saved, you need to compile it using the GCC compiler. This converts your C source code into a runnable executable.

Run the following command to compile client.c into an executable named client:
Bash
```bash
gcc serveur.c -o serveur
gcc client.c -o client
```

## 🚀 Step 3: Run the Client

Now that your code is compiled, you can execute it using the ./ command.

```bash
./serveur
./client
```
