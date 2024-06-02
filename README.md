# Crack Hash
Task from my university to crack MD5 hash using distributed brute-force algorithm.
So, here is a manager, who accepts HTTP requests, splits workload between workers and wait them to perform given tasks.
Each task is being saved into MongoDB replica set to assure consistency.
Each part of a task (as well as worker's result of brute-forcing) is to be stored into RabbitMQ message broker and delivered using ACKs.

### POST Request to upload crack hash task

POST /api/hash/crack

Request body:

```json
{
    "hash":"e2fc714c4727ee9395f324cd2e7f331f", 
    "max_length": 4
}
```
Task is to be splitted into N (taken from configuration) parts and distributed between workers. Server returns OK response if and only if task was successfully splitted, distributed (so, each worker ACKed its subtask) and saved in MongoDB replica set in all nodes (hardcoded to 3 -> one primary and two secondaries).

Response body:
```json
{
    "request_id":"730a04e6-4de9-41f9-9d5b-53b88b17afac"
}
```

### GET Request to get task's status

GET /api/hash/status?requestId=730a04e6-4de9-41f9-9d5b-53b88b17afac

Response body (if request is done):
```json
{
    "request_id": "730a04e6-4de9-41f9-9d5b-53b88b17afac",
    "status": "READY",
    "data": [
        "abcd"
    ]
}
```

The field "data" contains word which MD5 hashes are equal to requested hash with respect to specified "max_length" (or null, if there are no suitable words)

There are 3 statuses:
  * IN_PROGRESS - if not all the workers performed their work or result is not stored in the MongoDB replica set.
  * TIMEOUT - if request was performing too long (timeout specified in the configuration).
  * READY - if all the workers performed their work and result is stored in the MongoDB replica set.

# Dependencies

spdlog (copy of header-only version is in ./tp)

    Shared library makes impossible to get logger by its name.
    Linking static library is a pain-in-ass process.

nlohmann::json (copy of header-only version is in ./tp)

# How to launch

1. Clone repo

```bash
git clone ssh://git@github.com/lyect/crackhash.git
cd crackhash
```

2. Build base image

```bash
cd base-image
docker build -t crackhash:base .
```
3. Start MongoDB and RabbitMQ

```bash
cd ..
./build-compose.sh
```

4. Start manager's container
   
```bash
docker run -it --rm --mount type=bind,source=./test/manager,target=/home --network=crackhash_mongors-network -p 8080:80 crackhash:base
```

5. Build and run manager (in the container's terminal)

```bash
./manager-init.sh
./manager manager-config.json
```

6. Start first worker's container
   
```bash
docker run -it --rm --mount type=bind,source=./test/worker1,target=/home --network=crackhash_mongors-network crackhash:base
```

7. Build and run first worker (in the container's terminal)

```bash
./worker-init.sh
./worker worker-config.json
```

6. Start second worker's container
   
```bash
docker run -it --rm --mount type=bind,source=./test/worker2,target=/home --network=crackhash_mongors-network crackhash:base
```

7. Build and run second worker (in the container's terminal)

```bash
./worker-init.sh
./worker worker-config.json
```
