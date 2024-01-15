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

describe('Caching', () => {
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
  describe('Script', () => {
    it('should create html_*.h files if they are missing', async () => {
      // delete all html_*.h files
      const folderPath = 'wled00';
      let files = await fs.promises.readdir(folderPath);
      await Promise.all(files.map(file => {
        if (file.startsWith('html_') && path.extname(file) === '.h') {
          return fs.promises.unlink(path.join(folderPath, file));
        }
      }));

      // run script cdata.js and wait for it to finish
      process.env.NODE_ENV = 'production';
      await execPromise('node tools/cdata.js');

      // check if html_*.h files were created
      files = await fs.promises.readdir(folderPath);
      const htmlFiles = files.filter(file => file.startsWith('html_') && path.extname(file) === '.h');
      assert(htmlFiles.length > 0, 'html_*.h files were not created');
    });
  });
});