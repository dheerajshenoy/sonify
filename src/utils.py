import enum
import os

# Enum for size units
class SIZE_UNIT(enum.Enum):
   B = 1
   KB = 2
   MB = 3
   GB = 4

def convert_unit(size_in_bytes, unit):
   """ Convert the size from bytes to other units like KB, MB or GB"""
   if unit == SIZE_UNIT.KB:
       l = size_in_bytes / 1024
       if l > 1:
           return "{:.2f} KB".format(l)
   if unit == SIZE_UNIT.MB:
       l = size_in_bytes / (1024 * 1024)
       if l > 1:
           return "{:.2f} MB".format(l)
       else:
           l = size_in_bytes / 1024
           return "{:.2f} KB".format(l)
   elif unit == SIZE_UNIT.GB:
       l = size_in_bytes / (1024 * 1024 * 1024)
       if l > 1:
           return "{:.2f} GB".format(l)
       else:
           l = size_in_bytes / (1024 * 1024)
           return "{:.2f} MB".format(l)
   else:
       return size_in_bytes

def FileSize(file_name, size_type = SIZE_UNIT.MB ):
   """ Get file in size in given unit like KB, MB or GB"""
   size = os.path.getsize(file_name)
   return convert_unit(size, size_type)
