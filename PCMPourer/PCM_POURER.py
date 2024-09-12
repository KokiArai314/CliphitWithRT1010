import os
import wave
import shutil

explanation = [
  "**********************************************************",
  "                        PCM POURER                        ",
  "",
  "wavファイルを読み込み、Cソースファイル配列として書き出す",
  "対象Wavファイルの条件は以下",
  "   format      @wav",
  "   channel     @mono ",
  "   bit depth   @16bit ",
  "   sample rate @44.1kHz",
  "",
  "*********************************************@author arai**",
]
for line in explanation: print(line)

#environment variable
PCM_DATA_FOLDER = "PCM_DATA/"
PCM_DATA_FOLDER_ABS = (os.path.dirname(__file__) + PCM_DATA_FOLDER)
OUTPUT_FOLDER = "pcmdata"

DECLAREFILE_NAME = "declare_pcm_buffers"
DECLAREFILE_EXT = ".h"
DECLARE_TYPE = "static const int16_t"
DECLARE_VARIABLE_NAME = "pcm_buff_"
PCMFILE_NAME = "pcm_"
PCMFILE_EXT = ".txt"

def main():
  num_of_wavefiles = 0
  files_in_folder = os.listdir(PCM_DATA_FOLDER)
  wav_in_folder = []
  
  #Operate each files in pcm folder
  for file in files_in_folder:
    wave_file = isProperFormatWavFile(file) #format check
    if wave_file == False : 
      continue  #This file is not proper format
    else:
      wav_in_folder.append(wave_file) #Add file in list
    
  #Files acquired 
  if not wav_in_folder :
    print("There is no proper file.")
    return()
  else:
    print("There are "+ str(len(wav_in_folder))+" proper wav file(s)")
    print(wav_in_folder)
    
  writeDeclareWithWaveFiles(wav_in_folder)
  print("Program finish correctly.")
  return()
  

def isProperFormatWavFile(file):
  file_path = PCM_DATA_FOLDER + file
  
  # is wav file
  ext = os.path.splitext(file_path)[1][1:]
  if ext != "wav":
    return(False)
  
  # can be opened
  wave_file = wave.open(file_path, mode='rb')
  if not wave_file: 
    return(False)
  
  # is 16bit sample width
  if wave_file.getsampwidth() != 2 :
    wave_file.close()
    return(False)
  
  # is 44.1kHz sample rate
  if wave_file.getframerate() != 44100 :
    wave_file.close()
    return(False)
  
  # this wav file is proper format
  wave_file.close()
  return(file)

def writeDeclareWithWaveFiles(wav_in_folder):
  # make output folder
  try:
    os.mkdir(OUTPUT_FOLDER)
  except FileExistsError:
    # delete folder and rewrite
    print("Folder exists. Reset these pcm data.")
    shutil.rmtree(OUTPUT_FOLDER)
    os.mkdir(OUTPUT_FOLDER)
    pass
  
  # make buffer declare file in output folder
  declare_file = open(OUTPUT_FOLDER+'/'+DECLAREFILE_NAME+DECLAREFILE_EXT, 'w')
  # write header of file
  declare_file.write("#ifndef __DECLARE_PCM_BUFFER_H__\n")
  declare_file.write("#define __DECLARE_PCM_BUFFER_H__\n\n")
  declare_file.write("#include <stdint.h>\n\n")
  
  for index, wav in enumerate(wav_in_folder):
    # output files(declare and pcm data) with wav file
    writeDeclareFile(declare_file,index)
    openAndWritePCMFile(wav,index)
    
  # write footer of file
  declare_file.write("#endif\n")
  declare_file.close()
  return()

def writeDeclareFile(declare_file, index):
  padding_index = paddingIndex(index)
  declare_file.write(DECLARE_TYPE +" "+ DECLARE_VARIABLE_NAME + padding_index + "[]={\n")
  declare_file.write("  #include \"" + PCMFILE_NAME + padding_index + PCMFILE_EXT + "\" \n")
  declare_file.write("};\n\n")
  return 

def openAndWritePCMFile(wav, index):
  # open pcm file (destination)
  padding_index = paddingIndex(index)
  pcm_file = open(OUTPUT_FOLDER+'/' + PCMFILE_NAME + padding_index + PCMFILE_EXT, 'w')
  # open wav file (reading source)
  wave_file = wave.open(PCM_DATA_FOLDER + wav, mode='rb')
  wave_file.rewind()
  # write to pcm file from wav file
  num_of_sample = wave_file.getnframes()
  for sample in range(num_of_sample):
    hex2 = wave_file.readframes(1).hex()
    pcm_file.write("0x" + hex2[2:]+hex2[:2] +",\n") # little endian
  pcm_file.close()
  wave_file.close()
  return()
  
def paddingIndex(index):
  return "{:0=2}".format(index)

if __name__ == "__main__":
  main()
  print(".\n.\n")

