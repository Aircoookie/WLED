'use strict';
const assert = require('node:assert');
const { describe, it, before, after, afterEach, mock, test, run } = require('node:test');
const fs = require('fs');
const path = require('path');
const child_process = require('child_process');
const util = require('util');
const execPromise = util.promisify(child_process.exec);

process.env.NODE_ENV = 'test'; // Set the environment to testing
const cdata = require('./cdata.js');

describe('Functions', () => {
  const testFolderPath = path.join(__dirname, 'testFolder');
  const oldFilePath = path.join(testFolderPath, 'oldFile.txt');
  const newFilePath = path.join(testFolderPath, 'newFile.txt');

  // Create a temporary file before the test
  before(() => {
    fs.writeFileSync('temp.txt', 'Hello World');

    // Create test folder
    if (!fs.existsSync(testFolderPath)) {
      fs.mkdirSync(testFolderPath);
    }

    // Create an old file
    fs.writeFileSync(oldFilePath, 'This is an old file.');
    // Modify the 'mtime' to simulate an old file
    const oldTime = new Date();
    oldTime.setFullYear(oldTime.getFullYear() - 1);
    fs.utimesSync(oldFilePath, oldTime, oldTime);

    // Create a new file
    fs.writeFileSync(newFilePath, 'This is a new file.');
  });

  // delete the temporary file after the test
  after(() => {
    fs.unlinkSync('temp.txt');

    // Delete test folder
    if (fs.existsSync(testFolderPath)) {
      fs.rmSync(testFolderPath, { recursive: true });
    }
  });

  describe('isFileNewerThan', async () => {
    it('should return true if the file is newer than the provided time', async () => {
      const pastTime = Date.now() - 10000; // 10 seconds ago
      assert.strictEqual(cdata.isFileNewerThan('temp.txt', pastTime), true);
    });

    it('should return false if the file is older than the provided time', async () => {
      const futureTime = Date.now() + 10000; // 10 seconds in the future
      assert.strictEqual(cdata.isFileNewerThan('temp.txt', futureTime), false);
    });

    it('should return false if the file does not exist', async () => {
      assert.strictEqual(cdata.isFileNewerThan('nonexistent.txt', Date.now()), false);
    });
  });

  describe('isAnyFileInFolderNewerThan', async () => {
    it('should return true if a file in the folder is newer than the given time', async () => {
      const folderPath = path.join(__dirname, 'testFolder');
      const time = fs.statSync(path.join(folderPath, 'oldFile.txt')).mtime;
      assert.strictEqual(cdata.isAnyFileInFolderNewerThan(folderPath, time), true);
    });

    it('should return false if no files in the folder are newer than the given time', async () => {
      const folderPath = path.join(__dirname, 'testFolder');
      const time = new Date();
      assert.strictEqual(cdata.isAnyFileInFolderNewerThan(folderPath, time), false);
    });
  });
});

describe('General functionality', () => {
  const folderPath = 'wled00';
  const dataPath = path.join(folderPath, 'data');

  before(() => {
    process.env.NODE_ENV = 'production';
    fs.cpSync("wled00/data", "wled00Backup", { recursive: true });
  });
  after(() => {
    // Restore backup
    fs.rmSync("wled00/data", { recursive: true });
    fs.renameSync("wled00Backup", "wled00/data");
  });

  describe('Script', () => {
    it('should create html_*.h files if they are missing', async () => {
      // delete all html_*.h files
      let files = await fs.promises.readdir(folderPath);
      await Promise.all(files.map(file => {
        if (file.startsWith('html_') && path.extname(file) === '.h') {
          return fs.promises.unlink(path.join(folderPath, file));
        }
      }));

      // run script cdata.js and wait for it to finish
      await execPromise('node tools/cdata.js');

      // check if html_*.h files were created
      files = await fs.promises.readdir(folderPath);
      const htmlFiles = files.filter(file => file.startsWith('html_') && path.extname(file) === '.h');
      assert(htmlFiles.length > 0, 'html_*.h files were not created');
    });

    it('should rebuild if 1 or more html_*.h files are missing', async () => {
      // run script cdata.js and wait for it to finish
      await execPromise('node tools/cdata.js');

      // delete a random html_*.h file
      let files = await fs.promises.readdir(folderPath);
      let htmlFiles = files.filter(file => file.startsWith('html_') && path.extname(file) === '.h');
      if (htmlFiles.length > 0) {
        const randomFile = htmlFiles[Math.floor(Math.random() * htmlFiles.length)];
        await fs.promises.unlink(path.join(folderPath, randomFile));
      }

      // run script cdata.js and wait for it to finish
      await execPromise('node tools/cdata.js');

      // check if html_*.h files were created
      files = await fs.promises.readdir(folderPath);
      htmlFiles = files.filter(file => file.startsWith('html_') && path.extname(file) === '.h');
      assert(htmlFiles.length > 0, 'html_*.h files were not created');
    });

    it('should not rebuild if the files are already built', async () => {
      // delete all html_*.h files
      let files = await fs.promises.readdir(folderPath);
      await Promise.all(files.map(file => {
        if (file.startsWith('html_') && path.extname(file) === '.h') {
          return fs.promises.unlink(path.join(folderPath, file));
        }
      }));

      // run script cdata.js and wait for it to finish
      let startTime = Date.now();
      await execPromise('node tools/cdata.js');
      const firstRunTime = Date.now() - startTime;

      // run script cdata.js and wait for it to finish
      startTime = Date.now();
      await execPromise('node tools/cdata.js');
      const secondRunTime = Date.now() - startTime;

      // check if second run was faster than the first (must be at least 2x faster)
      assert(secondRunTime < firstRunTime / 2, 'html_*.h files were rebuilt');
    });

    it('should rebuild if a file changes', async () => {
      // run script cdata.js and wait for it to finish
      await execPromise('node tools/cdata.js');

      // modify index.htm
      fs.appendFileSync(path.join(dataPath, 'index.htm'), ' ');
      // delay for 1 second to ensure the modified time is different
      await new Promise(resolve => setTimeout(resolve, 1000));

      // run script cdata.js and wait for it to finish
      await execPromise('node tools/cdata.js');

      // check if html_ui.h was modified
      const stats = fs.statSync(path.join(folderPath, 'html_ui.h'));
      const modifiedTime = stats.mtimeMs;
      const currentTime = Date.now();
      assert(currentTime - modifiedTime < 500, 'html_ui.h was not modified');
    });
  });
});