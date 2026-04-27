AUTHORS:
Rafayel Amirkhanyan

Ryan Fusco
raf172@pitt.edu

Kieran Griperay
kjg86@pitt.edu

Build: sh build.sh
Run: sst --add-lib-path build config.py -- --program <programPath> --configuration <configPath> --output <outputPath>

Everything should be working, but testing this project was difficult due to its nature. We had issues with our pipeline stages that seemed
to have been causing most of the uncertainty with testing (sometimes instructions took shorter / longer than they should have when
there were no memory accesses). We noticed this issue was common when stalling, which led us to realize our pipeline was flipped.
