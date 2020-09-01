# Introduction
This folder contains the HTML user interface. The HTML user interface is compiled into the WLED firmware.

# Building

To make changes to the main `index.htm` or `index.js`, you will need to have [node](https://nodejs.org/) version 11 or later installed. From the root folder run these commands:

```
npm install
npm run webpack
npm run build
```

## npm Commands
The following section explains the usage of the commands above and when you need to execute them.

### npm install
This command will download and install all the required npm packages. After the npm packages are downloaded, you do not need to run this command again.  If pulling newer changes down that have newer packages, you may need to run this again.

### npm run webpack
This command will combine `index.htm` and `index.js` into `dist/index.html`. You will need to run this if you have made changes to `index.htm` or `index.js`. Note the files in the `dist` folder are not committed to source control.

### npm run build
This command will read and compress the .htm pages and `dist/index.html` into the C++ source code. Run this command when you are satisfied with the changes made to the source web page files. If you made changes to `index.htm` or `index.js`, you will need to run the webpack command above first.