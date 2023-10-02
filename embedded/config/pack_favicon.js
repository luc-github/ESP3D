const path = require("path");
const { createReadStream, createWriteStream } = require("fs");
const { createGzip } = require("zlib");
const faviconPath = path.normalize(__dirname + "/../assets/favicon.ico");
 
// Create a gzip function for reusable purpose
const compressFile = (filePath) => {
  const stream = createReadStream(filePath);
  stream
    .pipe(createGzip())
    .pipe(createWriteStream(`${filePath}.gz`))
    .on("finish", () =>console.log(`Successfully compressed the file at ${filePath}`)
    );
};
compressFile(faviconPath);