# 📚 **Advanced Database Organization**
## 🎯 **Assignment No. 1**



## 🏆 **Group No. 24**

### **Group Members:**
- **Mansi Patil**  
  *Student ID:* A20549858
- **Priyansh Salian**  
  *Student ID:* A20585026
- **Soham Joshi**  
  *Student ID:* A20586602


---

### 🛠️ **Group Members' Involvement**

- **Mansi Patil**
      - `void initStorageManager(void)`
      - `createPageFile()`
      - `closePageFile()`
      - `readFirstBlock()`
      - `readPreviousBlock()`

- **Priyansh Salian**
      - `openPageFile()`
      - `writeBlock()`
      - `readBlock()`
      - `getBlockPos()`
      - `appendEmptyBlock()`
      - `ensureCapacity()`

- **Soham Joshi**
      - `destroyPageFile()`
      - `readPreviousBlock()`
      - `readCurrentBlock()`
      - `readLastBlock()`
      - `writeCurrentBlock()`

---

### 📊 **Contribution Overview**

Each group member contributed equally to the assignment, with active participation and shared responsibilities throughout the project. 🎉

---

### 📂 **Files Included**

The folder includes the following files:
1. `dberror.c`
2. `dberror.h`
3. `Makefile`
4. `README.txt`
5. `storage_mgr.c`
6. `storage_mgr.h`
7. `test_assign1_1.c`
8. `test_helper.h`

---

### 📋 **Overview of the Storage Manager Implementation**

The Storage Manager is a tool that saves data from the computer's memory onto a disk and can also load data from the disk back into memory. It helps store and retrieve data efficiently, ensuring the system runs smoothly.

---

### 🚀 **How to Run the Code**

1. Open your terminal.
2. Clone the repository from BitBucket to your desired location using the command:
      ```bash
      git clone https://priyanshsalian-admin@bitbucket.org/priyanshsalian/fall_2024_24.git
      ```
3. Navigate to the cloned directory, which will be named Assignment 1:
      ```bash
      cd assignment-1
      ```
4. Compile the code by running:
      ```bash
      make
      ```
5. This will generate an executable file named `test_assign1.exe`.

6. Execute the test cases by running:
      ```bash
      ./test_assign1
      ```

---

### 🧩 Function Explanations

Below are the explanations of the functions used in our code:

#### 📄 Page File Manipulation Functions:

- **`void initStorageManager(void)`**

  Here, we set up the Storage Manager. This involves developing the page file and configuring the Storage Manager.

- **`createPageFile()`**

  The `createPageFile()` function checks if a file already exists and creates it if not. It allocates memory for the file, writes zeroes to it, and handles any errors during the process. If successful, it returns `RC_OK`.

- **`openPageFile()`**

  The `openPageFile()` function tries to open a file for reading and writing. If it can't open the file, it returns an error (`RC_FILE_NOT_FOUND`). If it opens successfully, it updates the file handle with information about the file's name, position, and total size, and then returns `RC_OK`.

- **`closePageFile()`**

  The `closePageFile()` function closes an open file. It checks if the file or file handle is invalid and returns an error if needed. If everything is fine, it closes the file and confirms whether it was successful.

- **`destroyPageFile()`**

  The `destroyPageFile()` function removes a page file using the provided `fileName` as a parameter. It returns `RC_OK` if the file is successfully deleted, or `RC_FILE_NOT_FOUND` if there is an issue.

#### 📖 Reading Functions:

- **`readBlock()`**

  The `readBlock()` function reads a specific page from a file into memory. It first checks if the file handle or memory page is valid. If either is not set up correctly, it prints "The file [fileName] could not be opened!" and returns `RC_FILE_HANDLE_NOT_INIT`. Then, it opens the file, seeks to the right position, and tries to read the page. If successful, it updates the current page position and closes the file, returning `RC_OK`. If reading fails or the file cannot be read, it returns `RC_READ_NON_EXISTING_PAGE`.

- **`getBlockPos()`**

  Returns the current page position from the file handle. If the file handle is missing, it shows an error message and returns `RC_FILE_HANDLE_NOT_INIT`.

- **`readFirstBlock()`**

  This function reads the first page of the file into memory. If the file handle or memory page is missing, it shows an error message and returns `RC_FILE_HANDLE_NOT_INIT`. If both are okay, it reads the first page and returns `RC_OK`.

- **`readPreviousBlock()`**

  Reads the page before the current one into memory. If the file handle or memory page is missing, it shows an error message and returns `RC_FILE_HANDLE_NOT_INIT`.

- **`readCurrentBlock()`**

  Reads the current page into memory. If the file handle or memory page is missing, it shows an error message and returns `RC_FILE_HANDLE_NOT_INIT`.

- **`readNextBlock()`**

  Reads the page after the current one into memory. If the file handle or memory page is missing, it shows an error message and returns `RC_FILE_HANDLE_NOT_INIT`.

- **`readLastBlock()`**

  Reads the last page of the file into memory. If the file handle or memory page is missing, it shows an error message and returns `RC_FILE_HANDLE_NOT_INIT`.

#### ✏️ Writing Functions:

- **`writeBlock()`**

  The `writeBlock()` function writes data to a specific page in a file. It first checks if the file handle or memory page is valid. It opens the file in write mode, seeks to the correct page position, and writes the data. It updates the current page position and recalculates the total number of pages in the file. If any step fails, it returns an error. If successful, it returns `RC_OK`.

- **`writeCurrentBlock()`**

  The `writeCurrentBlock()` function writes data to the current page of a file. It first checks if the file handle and memory page are valid. It retrieves the current page position, opens the file in write mode, and calls `writeBlock()` to perform the write operation. If the write is successful, it confirms the action; otherwise, it reports a failure. If any check fails, it returns an error.

- **`appendEmptyBlock()`**

  The `appendEmptyBlock()` function adds a new empty page to the end of a file. It first checks if the file handle is valid. It then seeks to the end of the file, allocates memory for the new page, and writes it to the file. If successful, it updates the file handle to reflect the new total number of pages and frees the allocated memory. If any step fails, it returns an error.

- **`ensureCapacity()`**

  The `ensureCapacity()` function ensures that a file has enough pages to meet the specified requirement. It first checks if the file handle is valid and if the number of pages requested is not negative. Then, it adds empty pages to the file until it has at least the requested number of pages. If successful, it returns `RC_OK`.

---

### 🧪 Test Functions that we have written

- #### `testManyPagesFiles()`
"We make a file with multiple pages, put different stuff on each page, and then check different pages to make sure everything's where it should be. This is like working with a big document making sure we can move between pages without losing or messing up any information."

- #### `testBlocksToReadWrite()`
"Here, we focus on the granular control of reading and writing to specific blocks within a page. We write data to particular blocks, then read from those same blocks to confirm the data is accurately stored. We also check surrounding blocks to ensure they remain unaffected, mimicking the ability to edit specific paragraphs in a document without altering the rest of the content."

- #### `testGuaranteeCapacity()`
"This test checks if our system can grow files when needed. We start with a file of a set size then keep adding stuff until we go over that size. The test works if the system adds more pages on its own to fit all the new stuff kind of like adding more paper to a folder when you run out of room."

- #### `testAbsentPage()`
"We intentionally attempt to read from a page number that doesn't exist in our file. The test is successful if our system handles this gracefully by returning an appropriate error message rather than crashing. This ensures our system is robust when dealing with user errors or invalid inputs."

- #### `testWriteAbsentPage()`
"Similar to the previous test, but we try to write to a non-existent page. We expect the system to recognize this invalid operation and respond with an error message, preventing any unintended side effects or data corruption."

- #### `testAddVacantBlock()`
"This test adds an empty block to our file and then verifies its emptiness. We're checking that our system can handle the concept of "blank" or "null" data, which is important for many file operations. It's akin to adding a blank page to a document and ensuring it's recognized as empty."

- #### `validatePageVolumeAndRandomNavigation()`
"We stress-test our file system by creating a large number of pages (potentially hundreds) and then randomly accessing different pages for reading and writing. This test ensures that our system maintains data integrity and performance even with large files and unpredictable access patterns."

- #### `checkOverwriteAndParallelFileOperations()`
"This test simulates a multi-user environment where several processes are writing to the same file simultaneously. We perform multiple write operations in parallel and then verify that all changes are correctly applied without data loss or corruption. This ensures our file system can handle concurrent access scenarios."

- #### `evaluateFileRemoveAndRecreate()`
"We delete a file and immediately create a new one with the same name. This test verifies that our file system properly releases resources when a file is deleted and correctly initializes a new file, even if it has the same name as a recently deleted one. We check various file operations after this process to ensure everything behaves as expected."

- #### `multipleFileOperationsCheck()`
"This comprehensive test performs a series of operations on multiple files: creating new files, opening existing ones, writing data, reading data, and closing files. We're verifying that our system can manage multiple files simultaneously without mixing up data or operations between files."

- #### `validateBlockAppendAndSequentialRead()`
"We test the ability to append new blocks of data to a file and then read through all blocks sequentially. This simulates adding new sections to a document and then reading it from start to finish, ensuring that all data is correctly stored and can be retrieved in the right order."

- #### `checkFileDamage()`
"This test intentionally corrupts part of our file data to simulate file damage. We then attempt various read and write operations to see how our system handles and potentially recovers from this damage. This helps ensure our system is resilient to unexpected data corruption."

- #### `checkConsistency()`
"We perform a series of random operations on our file (writes, reads, deletes) and then verify that the file's structure and content remain consistent and logical. This test ensures that no matter what sequence of operations we perform, our file system maintains data integrity and organizational structure."

- #### `testFileAppendToMaxCapacity()`
"We continuously append data to a file until we reach its maximum capacity. This test verifies how our system behaves when it can't add more data. We check for appropriate error messages and ensure that existing data remains intact when the capacity limit is reached."

- #### `testWriteFailureOnPowerLoss()`
"This test simulates a power failure or system crash during a write operation. We interrupt a write operation midway and then check the file's state after "power is restored." This helps ensure our system can recover from unexpected interruptions without data loss or file corruption."

- #### `testAccessFailureForInvalidBlock()`
"We attempt to access blocks that don't exist or are outside the valid range for our file. The test passes if our system correctly identifies these invalid accesses and raises appropriate errors, rather than returning garbage data or crashing. This helps ensure the robustness of our file system against invalid operations."

---

### 🙏 Gratitude

Thank you for getting involved in this project! Your time is truly appreciated. 🚀 🚀

---
