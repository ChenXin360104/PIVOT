# PIVOT System

This project presents the prototype system for the PIVOT technique proposed in our SIGMOD submission "Visualization-Oriented Progressive Time Series Transformation". It adopts a client-server architecture: the client and server are implemented using JavaScript and C++, respectively.

## Project Setup
To run this prototype system, execute both the **pivot-client** and **pivot-server** applications concurrently on the same local machine.

1. Download this project and `cd` to the root directory `${project_dir}`

2. Install the C++ dependencies
```sh
sudo apt install -y \
    cmake \            # CMake (>=3.20)
    clang \            # Clang compiler (>=14.0.0)
    libomp-dev \       # OpenMP runtime for Clang
    lldb \             # LLDB debugger
    lld \              # LLD linker
    libc++-dev \       # C++ standard library
    libc++abi-dev \    # C++ ABI library
    libunwind-dev      # Stack unwinding library
```

3. Compile PIVOT server
```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

4. Prepare data

* Create a `${project_dir}/data` directory and download the example datasets (csv files) from [OSF](https://osf.io/qugm3/files/osfstorage?view_only=58ea2143393245a69a4430188c0ef214)
  - Note: Due to their large size, some datasets are splitted. To restore the original csv file, such as nycsecond.csv, you should merge them by `cat  nycsecond.csv_*  >  nycsecond.csv`
* Transform the datasets (e.g., `nycdata`) to OM3 coefficients
```sh
cd ${project_dir}/build
./pivot-encode nycdata
```

5. Run PIVOT server
```sh
cd ${project_dir}/build
./pivot-server
```
Make sure the 3000 port is not occupied.

6. Run PIVOT client
```sh
cd ${project_dir}/pivot-client
npm install
npm run serve
```

After starting the client and server, you can visit http://localhost:8080/ in the browser.

You can visit this [google doc](https://docs.google.com/document/d/1RNhzro4HdUa7a9yOhelKUNIHlXKW6bUfBO6MKaLJH7E/edit?usp=sharing) to view instructions for using the system.

## Experiment
To run the experiment in our main paper, you should use Node.js, Python, and DuckDB.

1. Install Dependencies

* `cd` to the root directory `${project_dir}`
* Install Node.js (v16.20.2) following [the instructions](https://nodejs.org/en/download)
* Install the Node.js Client of DuckDB
```sh
npm install @duckdb/node-api
```
* Install Python and its dependencies
```sh
sudo apt install -y python3 pip3
pip3 install numpy matplotlib scikit-image
```

2. Create directories for storing experiment results
```sh
mkdir m4_result images compareresult output
```

### Cold-start Scenarios

1. Download necessary datasets required in the experiment from [OSF](https://osf.io/qugm3/files/osfstorage?view_only=58ea2143393245a69a4430188c0ef214)
```javascript
// see also `${project_dir}/scripts/applications.js`; lines 1763-1780
  datasets = [
      "sensordata 5,4,3,2,1,7,6"
      , "nycdata 7,9,1,2,3,4,5,6,8,10,11"
      , "soccerdata 5,3,1,2,4,6"
        ,"stockdata 9,5,1,2,3,4,6,7,8,10"
      //  ,"traffic 1,2,3,4,5,6,7,8,9,10"
      // , "synthetic1m 1,2,3,4,5"
      // , "synthetic2m 1,2,3,4,5"
      // , "synthetic4m 1,2,3,4,5"
      // ,"synthetic8m 1,2,3,4,5"
      // ,"synthetic16m 1,2,3,4,5"
      ,"synthetic32m 1,2,3,4,5"
      ,"synthetic64m 1,2,3,4,5"
      ,"synthetic128m 1,2,3,4,5"
      ,"synthetic256m 1,2,3,4,5"
      // ,"synthetic512m 1,2,3,4,5"
      //  ,"synthetic1b5v 1,2,3,4,5"
  ]
``` 

2. Run the DuckDB to obtain the baseline results
```sh
node scripts/applications.js static | grep experiment | awk 'BEGIN{print "experiment,table,function,width,time,memory(kb)"}{gsub(/,/, "I"); print $2,$5,$20,$23,$38,$41}' OFS="," > ../output/duck_static.csv
```

3. Compile PIVOT experiment and prepare data (if needed)
```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
./pivot-encode ${dataset_name} # e.g., nycdata
```

4. Run PIVOT experiment
```sh
./pivot-exp static "" "nycdata 7,9,1,2,3,4,5,6,8,10,11" 16
./pivot-exp static "" "soccerdata 5,3,1,2,4,6" 16
./pivot-exp static "" "sensordata 5,4,3,2,1,7,6" 16
./pivot-exp static "" "stockdata 9,5,1,2,3,4,6,7,8,10" 16
./pivot-exp static "" "synthetic32m 1,2,3,4,5" 16
./pivot-exp static "" "synthetic64m 1,2,3,4,5" 16
./pivot-exp static "" "synthetic128m 1,2,3,4,5" 16
./pivot-exp static "" "synthetic256m 1,2,3,4,5" 16
```

5. Collect experiment results
```sh
python3 scripts/createPhoto.py m4_result/ images/
python3 scripts/comparePhoto.py images/ compareresult/
ls compareresult/ | grep ours-cpp | awk -F'_' 'BEGIN{print "experiment,table,function,errorbound,width,time,ssim,memory(kb)"}{gsub(/,/, "I"); print $1,$2,$5,$10,$8,$14,$12,$16}' OFS=","| awk -F'.png' '{print $1}' > ./output/cpp_static.csv
```

### Interaction Scenarios
1. Download necessary datasets (nycsecond.csv, synthetic256m.csv) required in the experiment from [OSF](https://osf.io/qugm3/files/osfstorage?view_only=58ea2143393245a69a4430188c0ef214)
    - Note: Due to their large size, some datasets are splitted. To restore the original csv file, such as nycsecond.csv, you should merge them by `cat  nycsecond.csv_*  >  nycsecond.csv`

2. Run the DuckDB to obtain the baseline results
```sh
node scripts/applications.js interactions | grep experiment | awk 'BEGIN{print "experiment,table,function,width,time,memory(kb)"}{gsub(/,/, "I"); print $2,$5,$20,$23,$38,$41}' OFS="," > ./output/duck_interaction.csv
```

3. Compile PIVOT experiment and prepare data (if needed)
```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
./pivot-encode ${dataset_name} # e.g., nycdata
```

4. Run PIVOT experiment
```sh
./pivot-exp interactions ../scripts/plans.csv "interactions" 16
```

5. Collect experiment results
```sh
python3 scripts/createPhoto.py m4_result/ images/
python3 scripts/comparePhoto.py images/ compareresult/
ls compareresult/ | grep ours-cpp | awk -F'_' 'BEGIN{print "experiment,table,function,errorbound,width,time,ssim,memory(kb)"}{gsub(/,/, "I"); print $1,$2,$5,$10,$8,$14,$12,$16}' OFS=","| awk -F'.png' '{print $1}' > ./output/ours-cpp_interaction.csv
```