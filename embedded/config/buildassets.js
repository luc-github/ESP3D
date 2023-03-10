let path = require("path");
const fs = require("fs");
const { createReadStream, createWriteStream } = require("fs");
const { createGzip } = require("zlib");
const chalk = require("chalk");

let distPath = path.normalize(__dirname + "/../dist/");
let srcPath = path.normalize(__dirname + "/../assets/");
let headerPath = path.normalize(
  __dirname + "/../../esp3d/src/modules/http/favicon.h"
);



const convertToC = (filepath) => {
console.log(chalk.yellow("Converting bin to text file"));
//Cleaning files
if (fs.existsSync(distPath + "out.tmp")) fs.rmSync(distPath + "out.tmp");
if (fs.existsSync(distPath + "favicon.h")) fs.rmSync(distPath + "favicon.h");

const data = new Uint8Array(
  fs.readFileSync(filepath, { flag: "r" })
);
console.log("data size is ", data.length);
let out = "#define favicon_size  " + data.length + "\n";
out += "const unsigned char favicon[" + data.length + "] PROGMEM = {\n   ";
let nb = 0;
data.forEach((byte, index) => {
  out += " 0x" + (byte.toString(16).length == 1 ? "0" : "") + byte.toString(16);
  if (index < data.length - 1) out += ",";
  if (nb == 15) {
    out += "\n   ";
    nb = 0;
  } else {
    nb++;
  }
});

out += "\n};\n";
fs.writeFileSync(distPath + "out.tmp", out);

//Check conversion
if (fs.existsSync(distPath + "out.tmp")) {
  console.log(chalk.green("[ok]"));
} else {
  console.log(chalk.red("[error]Conversion failed"));
  return;
}

//Format header file
console.log(chalk.yellow("Building header"));
fs.writeFileSync(
  distPath + "favicon.h",
  fs.readFileSync(srcPath + "header.txt")
);
let bin2cfile = fs.readFileSync(distPath + "out.tmp").toString();
fs.appendFileSync(distPath + "favicon.h", bin2cfile);
fs.appendFileSync(
  distPath + "favicon.h",
  fs.readFileSync(srcPath + "footer.txt")
);

//Check format result
if (fs.existsSync(distPath + "favicon.h")) {
  console.log(chalk.green("[ok]"));
} else {
  console.log(chalk.red("[error]Conversion failed"));
  return;
}

//Move file to src
console.log(chalk.yellow("Overwriting header in sources"));
fs.writeFileSync(headerPath, fs.readFileSync(distPath + "favicon.h"));
if (fs.existsSync(headerPath)) {
  console.log(chalk.green("[ok]"));
} else {
  console.log(chalk.red("[error]Overwriting failed"));
  return;
}

}

 
// Create a gzip function for reusable purpose
const compressFile = (filePath, targetPath) => {
  const stream = createReadStream(filePath);
  stream
    .pipe(createGzip(targetPath))
    .pipe(createWriteStream(targetPath))
    .on("finish", () =>{console.log(`Successfully compressed  at ${targetPath}`);
    convertToC (targetPath)}
    );
};

compressFile(srcPath + "favicon.ico",  distPath + "favicon.ico.gz");