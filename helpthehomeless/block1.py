#barrystyle 28052019
import x25x_hash, os, sys, time, binascii

#result should be:
#block 0000759d927706394e22babf02515dce2dfab6b3668edb935de92cf552cee002
#nonce was 34180

def gennonce(decnonce):
  hexnonce = str(hex(decnonce)).replace('0x','')
  while len(hexnonce) < 8:
    hexnonce = '0' + hexnonce
  return str(hexnonce)

#main
header = '00112233'
while len(header) < 152:
  header = '00' + header

target = "0000ffff00000000000000000000000000000000000000000000000000000000"
targetbin = binascii.unhexlify(target)
nonce = 0
while True:
  
  complete_header = str(header) + str(gennonce(nonce))
  hashbin = x25x_hash.getPoWHash(binascii.unhexlify(complete_header))[::-1]

  if hashbin < targetbin:
     print 'block ' + str(binascii.hexlify(hashbin))
     print 'nonce was ' + str(nonce)
     sys.exit(0)

  if (nonce % 128 == 0):
     fnonce = str(hex(nonce)).replace('0x','')
     while len(fnonce) < 8:
      fnonce = '0' + fnonce
     print fnonce

  nonce += 1

