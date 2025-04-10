# VPT System

## Project Setup
To run this prototype system, execute both the **VPT-client** and **VPT-server** applications concurrently on the same local machine.


### VPT Client
```
1. cd VPT-client
2. npm install
3. npm run serve
```

### VPT Server
```
1. cd VPT-server
2. npm install
3. configure your db information in the initdb/dbconfig.json file
4. npm run start
```

### Run VPT

After starting the client and server, you can visit http://localhost:8081 in the browser. 

You can visit this [google doc](https://docs.google.com/document/d/1m2XpsjZgl6FHwlQK4e0V0yEHNjHLk6hpAQYcat0Fq-k/edit?usp=sharing) to view instructions for using the system.



### Enviroment
```
nodejs: v16.20.2
npm: 8.19.4
postgres: 14.12
```

### Data Preparation

We provide an example dataset, nycdata, that reflects the taxi activity in New York City. You can download the csv file from [Google Drive](https://drive.google.com/file/d/1Ab4erGlKN9NFXj8WhFHGy6XoaTH4kNKu/view?usp=sharing). We recommend using PostgreSQL for importing the database. 

#### Step 1: Import the Database
1、 Install PostgreSQL: If PostgreSQL is not already installed, download and install it from the [official website](https://www.postgresql.org/download/). We recommend using version 14.

2、 Import the Dataset: 
<!-- Use a graphical database tool such as Navicat or DBeaver to import the dataset into your local PostgreSQL database(Take Navicat for example): -->

<!-- (1) Create a postgres connection and database.
![Create connection](image-1.png)

(2) Import the CSV file into the public directory.
![Import csv file](image-2.png) -->
(1) Navigate to the Directory: Open your terminal and go to the /VPT-server/initdb directory:
```
cd /VPT-server/initdb  
```
(2) Install necessary dependencies：
```
npm install pg pg-copy-streams
```
(3) Run the Importing Script: Execute the copycsv.js script to import the dataset. 
```
node copycsv.js 
```
In this process, please replace the database configuration and file path in copycsv.js with your own:
```
const dbConfig = {    //update to your own
  host: 'localhost',         
  user: 'postgres',          
  database: 'postgres',   
  password: '******',     
  port: 5432,              
};                

const csvFilePath = '******/nycdata.csv'; //update to your own
```


3、Also, configure your database configuration in the initdb/dbconfig.json file：
   ```
   For example:
   "hostname":"127.0.0.1",
   "db":"postgres",         //update to your dbname
   "username":"postgres",    //update to your username
   "password":"******",      //update to your password
```

#### Step 2: Encode the Data
Ensure Node.js is installed on your system. If not, download and install it from the  [official website](https://nodejs.org/en/download). We recommend using version 16.

1、 Navigate to the Directory: Open your terminal and go to the /VPT-server/initdb directory:
```
cd /VPT-server/initdb  
```

2、 Run the Encoding Script: Execute the encode_bigarray_ave.js script to encode the data. This will generate the corresponding coefficient file in the database:
```
node encode_bigarray_ave.js 
```

You can also use your own dataset instead of `nycdata`, just make sure it consists of a single `t` column and multiple `v` columns. For example, the first five rows of `nycdata.csv` look like this:

```
"t","v1","v2","v3","v4","v5","v6","v7","v9","v10","v11"
0,13,0,8,6,1,0,1,1,86,11
1,17,0,11,11,5,0,2,3,105,12
2,34,0,8,9,6,0,2,1,129,7
3,23,0,6,9,5,0,4,2,164,11
4,32,0,9,9,6,0,2,0,203,8
```

