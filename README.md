# Recovery-Virtual-Memory

## Functions

### rvm_init()
Create back repo.

### rvm_map
Add seg into the list in the rvm. If the seg is in the list which means it is MAPPED. Other map request
will be aborted if the segname is the same.

return the seg address.

### rvm_unmap()
Unmap the file and the seg, free seg in the memory.

### rvm_destroy()
Destroy the back file.



