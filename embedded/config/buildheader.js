let path = require("path");
const fs = require("fs");
const child_process = require("child_process");
const chalk = require("chalk");

let distPath = path.normalize(__dirname + "/../dist/");
let srcPath = path.normalize(__dirname + "/../src/");
let headerPath = path.normalize(
  __dirname + "/../../esp3d/src/modules/http/embedded.h"
);

console.log(chalk.yellow("Converting bin to text file"));
//Cleaning files
if (fs.existsSync(distPath + "out.tmp")) fs.rmSync(distPath + "out.tmp");
if (fs.existsSync(distPath + "embedded.h")) fs.rmSync(distPath + "embedded.h");

//Convert bin2C
child_process.execSync(
  "bin2c -o " + distPath + "out.tmp" + " -m " + distPath + "index.html.gz"
);

//Check conversion
if (fs.existsSync(distPath + "out.tmp")) {
  console.log(chalk.green("[ok]"));
} else {
  console.log(chalk.red("[error]Conversion failed"));
  console.log(
    chalk.red(
      "Be sure bin2c executable is in your path (https://github.com/AraHaan/bin2c)"
    )
  );
  return;
}

//Format header file
console.log(chalk.yellow("Building header"));
fs.writeFileSync(
  distPath + "embedded.h",
  fs.readFileSync(srcPath + "header.txt")
);
let bin2cfile = fs.readFileSync(distPath + "out.tmp").toString();
let newfile = bin2cfile
  .replace("] ", "] PROGMEM ")
  .replace(/define.*dist_index_html_gz/, "define tool_html_gz")
  .replace(/char.*dist_index_html_gz/, "char tool_html_gz");
fs.appendFileSync(distPath + "embedded.h", newfile);
fs.appendFileSync(
  distPath + "embedded.h",
  fs.readFileSync(srcPath + "footer.txt")
);

//Check format result
if (fs.existsSync(distPath + "embedded.h")) {
  console.log(chalk.green("[ok]"));
} else {
  console.log(chalk.red("[error]Conversion failed"));
  return;
}

//Move file to src
console.log(chalk.yellow("Overwriting header in sources"));
fs.writeFileSync(headerPath, fs.readFileSync(distPath + "embedded.h"));
if (fs.existsSync(headerPath)) {
  console.log(chalk.green("[ok]"));
} else {
  console.log(chalk.red("[error]Overwriting failed"));
  return;
}
