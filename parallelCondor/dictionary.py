import sys
import hashlib

global passwd
passwd = 'abb45fdc12b3e45df9640ac76fa1ccf8'
stringln = 8
string = "aaaaaaaa"
newstring = ""
h = 0
lower = ((stringln**26 /1024) * worker)
upper = ((stringln**26 /1024) * (worker + 1)) - 1
addamount = []
i = lower
ltrs = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', \
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z']
breakout = False

def main():
    while i != upper + 1:
        for let0 in ltrs[(ord(string[0]) - ord('a')):(ord(string[0]) - ord('a'))- 1]:
            for let1 in ltrs[(ord(string[1]) - ord('a')):(ord(string[1]) - ord('a'))- 1]:
                for let2 in ltrs[(ord(string[2]) - ord('a')):(ord(string[2]) - ord('a'))- 1]:
                    for let3 in ltrs[(ord(string[3]) - ord('a')):(ord(string[3]) - ord('a'))- 1]:
                        for let4 in ltrs[(ord(string[4]) - ord('a')):(ord(string[4]) - ord('a'))- 1]:
                            for let5 in ltrs[(ord(string[5]) - ord('a')):(ord(string[5]) - ord('a'))- 1]:
                                for let6 in ltrs[(ord(string[6]) - ord('a')):(ord(string[6]) - ord('a'))- 1]:
                                    for let7 in ltrs[(ord(string[7]) - ord('a')):(ord(string[7]) - ord('a'))- 1]:
                                        phrase = ltr0 + ltr1 + ltr2 + ltr3 + ltr4 + ltr5 + ltr6 + ltr7 
                                        encryptphrase = hashlib.md5(phrase).hexdigest()
                                        if str(encryptphrase) == passwd:
                                            print 'Match:', encryptphrase
                                            sys.exit(0)

                                            
if __name__ == "__main__":
    global worker
    worker = int(sys.argv[1])
    main()






