/**
 * ThermoGo AR — Image Target Compiler
 *
 * Converts your poster image (JPG/PNG) into the .mind file that
 * MindAR uses for image tracking.
 *
 * HOW TO RUN:
 *   1. Export your poster as a JPG and save it in this folder as  poster.jpg
 *   2. Open a terminal here and run:
 *        npm install
 *        node compile-target.js
 *   3. A file called  targets/poster.mind  will be created.
 *   4. That's it — open index.html in a browser (served via HTTPS).
 */

const path     = require('path');
const fs       = require('fs');
const compiler = require('mind-ar/src/image-target/compiler');

(async () => {
  const inputPath  = path.join(__dirname, 'poster.jpg');
  const outputPath = path.join(__dirname, 'targets', 'poster.mind');

  if (!fs.existsSync(inputPath)) {
    console.error('\n  ERROR: poster.jpg not found.\n');
    console.error('  Export your poster as a JPG file and place it in this folder as:\n');
    console.error('    poster.jpg\n');
    process.exit(1);
  }

  console.log('\n  ThermoGo AR — compiling image target...\n');

  try {
    const { createCanvas, loadImage } = require('canvas');

    const img    = await loadImage(inputPath);
    const canvas = createCanvas(img.width, img.height);
    const ctx    = canvas.getContext('2d');
    ctx.drawImage(img, 0, 0);

    const imageData = ctx.getImageData(0, 0, img.width, img.height);

    const c = new compiler.Compiler();
    await c.compileImageTargets([imageData], (progress) => {
      process.stdout.write(`\r  Progress: ${(progress * 100).toFixed(1)}%  `);
    });

    const buffer = await c.exportData();
    fs.writeFileSync(outputPath, Buffer.from(buffer));

    console.log('\n\n  Done!  targets/poster.mind created successfully.');
    console.log('  Upload the whole folder to Netlify Drop (netlify.com/drop) and scan the QR code.\n');
  } catch (err) {
    console.error('\n  Compilation failed:', err.message);
    console.error('\n  Tip: try the browser-based compiler instead:');
    console.error('  https://hiukim.github.io/mind-ar-js-doc/tools/compile\n');
  }
})();
