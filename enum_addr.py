import csv 
import os

USER = os.getlogin()
FILE_CSV = "MAC.CSV"
FILE_PATH_CSV = "/home/"+ USER + "/Documents/" + FILE_CSV

THIS_BASE = os.path.basename(__file__)
FILE_H = os.path.splitext(THIS_BASE)[0] + ".h"

def build_header(file_h, file_csv):
	header = open(file_h,'w')
	header.truncate()

	header.write("#ifndef " + file_h.replace(".","_").upper() + "\n" )
	header.write("const unsigned char apple_macs[][3] = { \n")
	csv_to_header(file_csv, header)
	header.write("};\n")
	header.write("#endif\n")
	

def csv_to_header(file, open_file):
	if os.path.isfile(file) == False:
		print "Error: "+ file + " does not exist!"
		return -1

	csvfile = open(file, 'rb')
	apple_macs = list(csv.DictReader(csvfile))
	max_num = len(apple_macs)
	#print max_num
	for count,row in enumerate(apple_macs):
		mac_str = row['Assignment']
		mac_rev_arr = [ "0x"+mac_str[i:i+2] for i  in xrange(0, len(mac_str),2)][::-1]
		mac_cform = ", ".join(mac_rev_arr)
		if count == max_num-1:
			open_file.write(" { "+mac_cform+" } \n")
		else:
			open_file.write(" { "+mac_cform+" }, \n")





if __name__ == '__main__':
	
	"""
	print FILE_H
	print "#ifndef " + FILE_H.replace(".","_").upper() 
	csv_to_header(FILE_PATH_CSV,"")
	"""

	build_header(FILE_H,FILE_PATH_CSV)

