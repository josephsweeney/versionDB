# TODO

* Store hash_str in ref



# Data flow

How data is added:
1. Take a name and some data
2. Hash name and data, write to object database
3. Create commit object that points to previous commit for this name and the hash
   * Commit includes unix epoch timestamp
4. Store commit in object database
5. Update ref for that name

How current data is retrieved:
1. Get commit hash for name from ref
2. Get data hash from commit
3. Read and return data

How past data is retrieved:
1. Get commit hash for name from ref
2. Traverse commits until you find the first commit with timestamp less than given timestamp
3. Get data hash from that commit
4. Read and return data

