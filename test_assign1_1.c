#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// test name
char *testName;

/* test output files */
#define TESTPF "test_pagefile.bin"

/* prototypes for test functions */
static void testCreateOpenClose(void);
static void testSinglePageContent(void);
static void testManyPagesFiles(void);
static void testBlocksToReadWrite(void);
static void testGuaranteeCapacity(void);
static void testAbsentPage(void);
static void testWriteAbsentPage(void);
static void testAddVacantBlock(void);
static void validatePageVolumeAndRandomNavigation(void);
static void checkOverwriteAndParallelFileOperations(void);
static void evaluateFileRemoveAndRecreate(void);
static void multipleFileOperationsCheck(void);
static void validateBlockAppendAndSequentialRead(void);
static void checkFileDamage(void);
static void checkConsistency(void);
static void assessFileAppendToMaxCapacity(void);
static void testWriteFailureOnPowerLoss(void);
static void testAccessFailureForInvalidBlock(void);

/* main function running all tests */
int main (void)
{
  testName = "";
  
  initStorageManager();

  testCreateOpenClose();
  testSinglePageContent();
  testManyPagesFiles();
  testBlocksToReadWrite();
  testGuaranteeCapacity();
  testAbsentPage();
  testWriteAbsentPage();
  testAddVacantBlock();
  validatePageVolumeAndRandomNavigation();
  checkOverwriteAndParallelFileOperations();
  evaluateFileRemoveAndRecreate();
  multipleFileOperationsCheck();
  validateBlockAppendAndSequentialRead();
  checkFileDamage();
  checkConsistency();
  assessFileAppendToMaxCapacity();
  testWriteFailureOnPowerLoss();
  testAccessFailureForInvalidBlock();
  return 0;
}


/* check a return code. If it is not RC_OK then output a message, error description, and exit */
/* Try to create, open, and close a page file */
void
testCreateOpenClose(void)
{
  SM_FileHandle fh;

  testName = "test create open and close methods";

  TEST_CHECK(createPageFile (TESTPF));

  TEST_CHECK(openPageFile (TESTPF, &fh));
  ASSERT_TRUE(strcmp(fh.fileName, TESTPF) == 0, "filename correct");
  ASSERT_TRUE((fh.totalNumPages == 1), "expect 1 page in new file");
  ASSERT_TRUE((fh.curPagePos == 0), "freshly opened file's page position should be 0");
  //
  TEST_CHECK(closePageFile (&fh));
  TEST_CHECK(destroyPageFile (TESTPF));
  //
  // // after destruction trying to open the file should cause an error
  ASSERT_TRUE((openPageFile(TESTPF, &fh) != RC_OK), "opening non-existing file should return an error.");

  TEST_DONE();
}

/* Try to create, open, and close a page file */
void
testSinglePageContent(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test single page content";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // create a new page file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("created and opened file\n");

  // read first page into handle
  TEST_CHECK(readFirstBlock (&fh, ph));
  // the page should be empty (zero bytes)
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page of freshly initialized page");
  printf("first block was empty\n");

  // change ph to be a string and write that one to disk
  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = (i % 10) + '0';
  TEST_CHECK(writeBlock (0, &fh, ph));
  printf("writing first block\n");

  // read back the page containing the string and check that it is correct
  TEST_CHECK(readFirstBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");
  printf("reading first block\n");

  // destroy new page file
  TEST_CHECK(destroyPageFile (TESTPF));

  TEST_DONE();
}

/* Handling multiple pages. */
void testManyPagesFiles(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test multiple pages in file";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // Now we are creating a new page file and opening it
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));
  printf("File is created and opened\n");
  // First page is being written
  for (i = 0; i < PAGE_SIZE; i++) {
    ph[i] = 'V' + (i % 26);
  }

  TEST_CHECK(writeBlock(0, &fh, ph));
  printf("First block has been written\n");
  // Adding 2 more pages
  TEST_CHECK(appendEmptyBlock(&fh));
  TEST_CHECK(appendEmptyBlock(&fh));
  // Ensuring the capacity of the file
  TEST_CHECK(ensureCapacity(3, &fh));

  ASSERT_TRUE(fh.totalNumPages == 3, "After two empty pages are appended the expectation was that the total no of pages will be 3");

  // Writing to the second and third pages on the file
  for (i = 0; i < PAGE_SIZE; i++)
    ph[i] = 'V' + (i % 26);
  TEST_CHECK(writeBlock(1, &fh, ph));
  for (i = 0; i < PAGE_SIZE; i++)
    ph[i] = 'V' + (i % 26);
  TEST_CHECK(writeBlock(2, &fh, ph));


  // The contents of the first file is being read to verify the data
  TEST_CHECK(readFirstBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 'V' + (i % 26)), "Correct values were expected in the first page");
  printf("reading first block\n");

  TEST_CHECK(readBlock(1, &fh, ph));
  for (i = 0; i < PAGE_SIZE; i++)
    ASSERT_TRUE(ph[i] == 'V' + (i % 26), "Correct values were expected in the second page");

  TEST_CHECK(readBlock(2, &fh, ph));
  for (i = 0; i < PAGE_SIZE; i++)
    ASSERT_TRUE(ph[i] == 'V' + (i % 26), "Correct values were expected in the third page");

  TEST_CHECK(closePageFile(&fh));
  TEST_CHECK(destroyPageFile(TESTPF));

  free(ph);

  TEST_DONE();
}

/* Test: Here we are concerned with specific blocks in a page. */
void testBlocksToReadWrite(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "Specific blocks will be read and written in the block";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // Now we are creating a new page file and opening it
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));
  printf("File is created and opened\n");

  // Page 0 is being written
  for (i = 0; i < PAGE_SIZE; i++)
    ph[i] = 'K';
  TEST_CHECK(writeBlock(0, &fh, ph));

  // Anew page is being appended and written (page 1)
  TEST_CHECK(appendEmptyBlock(&fh));
  for (i = 0; i < PAGE_SIZE; i++)
    ph[i] = 'G';
  TEST_CHECK(writeBlock(1, &fh, ph));

  // First page is being read and checked
  TEST_CHECK(readBlock(0, &fh, ph));
  for (i = 0; i < PAGE_SIZE; i++)
    ASSERT_TRUE(ph[i] == 'K', "expecting 'K' in first page");

  // First page is being read and checked
  TEST_CHECK(readBlock(1, &fh, ph));
  for (i = 0; i < PAGE_SIZE; i++)
    ASSERT_TRUE(ph[i] == 'G', "expecting 'G' in second page");

  TEST_CHECK(closePageFile(&fh));
  TEST_CHECK(destroyPageFile(TESTPF));

  free(ph);

  TEST_DONE();
}

/* Test: This test checks if we can add more pages to our file when needed. */
void testGuaranteeCapacity(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test to guarantee capacity";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // Now we are creating a new page file and opening it
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));
  printf("File is created and opened\n");

  // File should have capacity of 5
  TEST_CHECK(ensureCapacity(5, &fh));
  ASSERT_TRUE(fh.totalNumPages ==5, "5 pages are expected after ensureCapacity function runs with no of pages as 5 in parameter ");

  // Page 4 is being written
  for (i = 0; i < PAGE_SIZE; i++)
    ph[i] = 'H';
  TEST_CHECK(writeBlock(4, &fh, ph));

  // Reading the first block and checking it
  TEST_CHECK(readBlock(4, &fh, ph));
  for (i = 0; i < PAGE_SIZE; i++)
    ASSERT_TRUE(ph[i] == 'H', "expecting 'H' in 5th page");

  TEST_CHECK(closePageFile(&fh));
  TEST_CHECK(destroyPageFile(TESTPF));

  free(ph);

  TEST_DONE();
}

/* Test: Ensuring our system doesn't crash when we try to read a page that doesn't exist. Instead, it should give us a proper error message. */
void testAbsentPage(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  testName = "test to read absent page";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // Now we are creating a new page file and opening it
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));
  printf("File is created and opened\n");

  // Reading page 1 that is not present or does not exist
  ASSERT_TRUE(readBlock(1, &fh, ph) == RC_READ_NON_EXISTING_PAGE, "reading non-existing page should return an error");

  TEST_CHECK(closePageFile(&fh));
  TEST_CHECK(destroyPageFile(TESTPF));

  free(ph);

  TEST_DONE();
}

/* Test: Trying to write to a page that does not exist. */
void testWriteAbsentPage(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test to check weather write to absent page is possible or not";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // Now we are creating a new page file and opening it
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));
  printf("File is created and opened\n");

  // Writing to absent page 2 the value X.
  for (i = 0; i < PAGE_SIZE; i++)
    ph[i] = 'X';
  ASSERT_TRUE(writeBlock(2, &fh, ph) == RC_WRITE_FAILED, "Error is expected when writing to absent page is performed.");

  TEST_CHECK(closePageFile(&fh));
  TEST_CHECK(destroyPageFile(TESTPF));

  free(ph);

  TEST_DONE();
}

/* Test: This test will append a new block that will be empty to our file. */
void testAddVacantBlock(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;

  testName = "Test to check if vacant block can be added or not.";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // Now we are creating a new page file and opening it
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));
  printf("File is created and opened\n");

  // Add a vacant block
  TEST_CHECK(appendEmptyBlock(&fh));
  ASSERT_TRUE(fh.totalNumPages == 2, "total number of pages should be 2 after adding a vacant block.");

  // Read values of newly added block at first and check weather it's empty or not
  TEST_CHECK(readBlock(1, &fh, ph));
  for (int i = 0; i < PAGE_SIZE; i++)
    ASSERT_TRUE(ph[i] == 0, "Block should be vacant.");

  TEST_CHECK(closePageFile(&fh));
  TEST_CHECK(destroyPageFile(TESTPF));

  free(ph);

  TEST_DONE();
}

/* Test: Creating hundreds of pages and jumping around to make sure that everything remains consistent.  */
void validatePageVolumeAndRandomNavigation(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i, varPageI;

  testName = "Validate Page volume and random navigation";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // Now we are creating a new page file and opening it
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));
  printf("File is created and opened\n");

  // Guarantee the no of pages.
  TEST_CHECK(ensureCapacity(1000, &fh));
  ASSERT_TRUE(fh.totalNumPages == 1000, "1000 pages should be there after running ensureCapacity function.");

  // Initialized an array to track which page is written
  int pagesWritten[1000] = {0};  // All have values as zero

  // Writing random data to random pages
  for (i = 0; i < 500; i++) {
    varPageI = rand() % 1000;  // Page no can be any no between 0 and 999
    for (int j = 0; j < PAGE_SIZE; j++)
      ph[j] = 'D' + (j % 26);
    TEST_CHECK(writeBlock(varPageI, &fh, ph));
    pagesWritten[varPageI] = 1;  // Marking this page as written
  }

  // Read values of newly added block at first and check its values
  for (i = 0; i < 500; i++) {
    do {
      varPageI = rand() % 1000;  // Random page between 0 and 999
    } while (pagesWritten[varPageI] == 0);  // Skip pages that weren't written

    TEST_CHECK(readBlock(varPageI, &fh, ph));
    for (int j = 0; j < PAGE_SIZE; j++)
      ASSERT_TRUE(ph[j] == 'D' + (j % 26), "while random access data should be correct.");
  }

  TEST_CHECK(closePageFile(&fh));
  TEST_CHECK(destroyPageFile(TESTPF));

  free(ph);

  TEST_DONE();
}


/* Test: Checking overwrite and parallel file operation.  */
void checkOverwriteAndParallelFileOperations(void)
{
  SM_FileHandle fh1, fh2;
  SM_PageHandle ph;
  int i;

  testName = "check Overwrite File Operations";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // Now we are creating a new page file and opening it with two handles
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh1));
  TEST_CHECK(openPageFile(TESTPF, &fh2));

  // Write different data using each file handle to the same page
  for (i = 0; i < PAGE_SIZE; i++)
    ph[i] = 'O';
  TEST_CHECK(writeBlock(0, &fh1, ph));

  for (i = 0; i < PAGE_SIZE; i++)
    ph[i] = 'I';
  TEST_CHECK(writeBlock(0, &fh2, ph));

  // First file handle is being read  to verify the value
  TEST_CHECK(readBlock(0, &fh1, ph));
  for (i = 0; i < PAGE_SIZE; i++)
    ASSERT_TRUE(ph[i] == 'I', "expect 'I' from second handle's write");

  TEST_CHECK(closePageFile(&fh1));
  TEST_CHECK(closePageFile(&fh2));
  TEST_CHECK(destroyPageFile(TESTPF));

  free(ph);

  TEST_DONE();
}

/* Test: Deleting a document and afterwards making a new one with the same name. */
void evaluateFileRemoveAndRecreate(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test to evaluate File Remove And Recreate";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // Now we are creating a new page file and opening it
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));
  for (i = 0; i < PAGE_SIZE; i++)
    ph[i] = 'X';
  TEST_CHECK(writeBlock(0, &fh, ph));
  TEST_CHECK(closePageFile(&fh));

  // Destroying file that was just created.
  TEST_CHECK(destroyPageFile(TESTPF));

  // Re-creating file with the same name that was deleted
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));

  // File should start with empty page
  TEST_CHECK(readBlock(0, &fh, ph));
  for (i = 0; i < PAGE_SIZE; i++)
    ASSERT_TRUE(ph[i] == 0, "file should be empty");

  TEST_CHECK(closePageFile(&fh));
  TEST_CHECK(destroyPageFile(TESTPF));

  free(ph);

  TEST_DONE();
}

/* Test: Test for multitasking to check creation, opening, writing, reading and closing multiple files to make sure that everything remains intact. */
void multipleFileOperationsCheck(void)
{
  SM_FileHandle fh1, fh2;
  SM_PageHandle ph;
  int i;

  testName = "multiple File Operations Check";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // Now we are creating two new page files
  TEST_CHECK(createPageFile("testfile1.bin"));
  TEST_CHECK(createPageFile("testfile2.bin"));

  // Opening the new page files
  TEST_CHECK(openPageFile("testfile1.bin", &fh1));
  TEST_CHECK(openPageFile("testfile2.bin", &fh2));

  // Different data is being written in the page files
  for (i = 0; i < PAGE_SIZE; i++)
    ph[i] = 'M';
  TEST_CHECK(writeBlock(0, &fh1, ph));

  for (i = 0; i < PAGE_SIZE; i++)
    ph[i] = 'N';
  TEST_CHECK(writeBlock(0, &fh2, ph));

  // Read back the data from each file and verify
  TEST_CHECK(readBlock(0, &fh1, ph));
  for (i = 0; i < PAGE_SIZE; i++)
    ASSERT_TRUE(ph[i] == 'M', "expect 'A' in first file");

  TEST_CHECK(readBlock(0, &fh2, ph));
  for (i = 0; i < PAGE_SIZE; i++)
    ASSERT_TRUE(ph[i] == 'N', "expect 'B' in second file");

  TEST_CHECK(closePageFile(&fh1));
  TEST_CHECK(closePageFile(&fh2));

  // Delete the files
  TEST_CHECK(destroyPageFile("testfile1.bin"));
  TEST_CHECK(destroyPageFile("testfile2.bin"));

  free(ph);

  TEST_DONE();
}

/* Test: We're testing if we can add new sections to our file and then read through them in order.*/
void validateBlockAppendAndSequentialRead(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i, varSumP = 10;

  testName = "validate Block Append And Sequential Read";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // Now we are creating new page and opening it
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));

  // Add 10 pages with different data and check its values
  for (i = 0; i < varSumP; i++) {
    for (int j = 0; j < PAGE_SIZE; j++)
      ph[j] = 'A' + i;  // Use distinct letters in every page
    TEST_CHECK(appendEmptyBlock(&fh));
    TEST_CHECK(writeBlock(i, &fh, ph));
  }

  // Sequentially read each page and check values
  for (i = 0; i < varSumP; i++) {
    TEST_CHECK(readBlock(i, &fh, ph));
    for (int j = 0; j < PAGE_SIZE; j++)
      ASSERT_TRUE(ph[j] == 'A' + i, "expect correct data in appended pages");
  }

  TEST_CHECK(closePageFile(&fh));
  TEST_CHECK(destroyPageFile(TESTPF));

  free(ph);

  TEST_DONE();
}

/* Test: This test simulates what happens if our file gets corrupted. */
void checkFileDamage(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "check File Damage";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // Now we are creating a new page file and opening it
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));

  // Writing incomplete data to make the situation of file damage.
  for (i = 0; i < PAGE_SIZE / 2; i++)
    ph[i] = 'X';
  for (i = PAGE_SIZE / 2; i < PAGE_SIZE; i++)
    ph[i] = 0;  // Remaining part is set to zero
  TEST_CHECK(writeBlock(0, &fh, ph));

  // Verify page corruption by reading the page.
  TEST_CHECK(readBlock(0, &fh, ph));
  for (i = 0; i < PAGE_SIZE / 2; i++)
    ASSERT_TRUE(ph[i] == 'X', "First page should have valid data.");
  for (i = PAGE_SIZE / 2; i < PAGE_SIZE; i++)
    ASSERT_TRUE(ph[i] == 0, "data is zeroed or corrupted in second half of page");

  TEST_CHECK(closePageFile(&fh));
  TEST_CHECK(destroyPageFile(TESTPF));

  free(ph);

  TEST_DONE();
}

/* Test: We're making sure our file stays consistent while continuous writes. */
void checkConsistency(void)
{
  SM_FileHandle fh;
  SM_PageHandle PageHandleWrite, PageHandleRead;
  int i;

  testName = "check consistency";

  PageHandleWrite = (SM_PageHandle) malloc(PAGE_SIZE);
  PageHandleRead = (SM_PageHandle) malloc(PAGE_SIZE);

  // Now we are creating a new page file and opening it
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));

  // Writing data
  for (i = 0; i < PAGE_SIZE; i++)
    PageHandleWrite[i] = 'P';  // Use 'W' to mark written data

  for (i = 0; i < 100; i++) {
    // Write a character to the block
    TEST_CHECK(writeBlock(0, &fh, PageHandleWrite));

    // simultaneously read the block and check its value.
    TEST_CHECK(readBlock(0, &fh, PageHandleRead));
    for (int j = 0; j < PAGE_SIZE; j++)
      ASSERT_TRUE(PageHandleRead[j] == 'P',"expect consistency in data");
  }

  TEST_CHECK(closePageFile(&fh));
  TEST_CHECK(destroyPageFile(TESTPF));

  free(PageHandleWrite);
  free(PageHandleRead);

  TEST_DONE();
}

/* Test: This test tries to determine what happens if we attempt to put more information into our file than it can contain. */
void assessFileAppendToMaxCapacity(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i, varSumP = 1000000; // Adjust this value to simulate a large file size limit

  testName = "assess File Append To Max Capacity";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // Now we are creating a new page file and opening it
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));

  // Add 1000000 pages.
  for (i = 0; i < varSumP; i++) {
    memset(ph, 'A' + (i % 26), PAGE_SIZE);  // Page is being filled
    int result = appendEmptyBlock(&fh);
    if (result != RC_OK) {
      printf("Limit is reached of file system due to %d pages.\n", fh.totalNumPages);
      break;
    }
  }

  // check if the file reached its limit.
  ASSERT_TRUE(fh.totalNumPages == i+1, "expect the total number of pages to match the append operations");

  TEST_CHECK(closePageFile(&fh));
  TEST_CHECK(destroyPageFile(TESTPF));

  free(ph);

  TEST_DONE();
}


/* Test: We're simulating a power failure while writing to our file. */
void testWriteFailureOnPowerLoss(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test Write Failure On Power Loss";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // Now we are creating a new page file and opening it
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));

  // Page is being written
  for (i = 0; i < PAGE_SIZE; i++)
    ph[i] = 'X';

  // Replicate a power loss by halting the write procedure
  TEST_CHECK(writeBlock(0, &fh, ph));
  // Abruptly closing the file to copy power loss
  TEST_CHECK(closePageFile(&fh));

  // Open file and check integrity.
  TEST_CHECK(openPageFile(TESTPF, &fh));
  TEST_CHECK(readBlock(0, &fh, ph));

  for (i = 0; i < PAGE_SIZE; i++)
    ASSERT_TRUE(ph[i] == 'X', "Ensure data integrity post-recovery");

  TEST_CHECK(closePageFile(&fh));
  TEST_CHECK(destroyPageFile(TESTPF));

  free(ph);

  TEST_DONE();
}

/* Test: This test will ensure that when attempting to access a portion of the file that does not exist or is invalid, an error is raised. */
void testAccessFailureForInvalidBlock(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int noNotValid = -1; // Take a negative no to showcase out of bound page.

  testName = "test Access Failure For Invalid Block";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // Now we are creating a new page file and opening it
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));

  // Reading in invalid block
  int result = readBlock(noNotValid, &fh, ph);
  ASSERT_TRUE(result == RC_READ_NON_EXISTING_PAGE, "Read failure should occur");

  // Writing in invalid block
  result = writeBlock(noNotValid, &fh, ph);
  ASSERT_TRUE(result == RC_WRITE_FAILED, "Write failure should occur");

  TEST_CHECK(closePageFile(&fh));
  TEST_CHECK(destroyPageFile(TESTPF));

  free(ph);

  TEST_DONE();
}
