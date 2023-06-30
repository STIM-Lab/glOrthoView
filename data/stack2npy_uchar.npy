import numpy as np
from skimage import io
import os

# load the images in the /stack directory to a NumPy array

# output that NumPy array in two different formats: unsigned char, float
# make sure to name them two different things (stack_float.npy, stack_uchar.npy)

# add those files to the repository and push



def load_images_uchar(directory):
  images = []
  for filename in os.listdir(directory):
    image = io.imread(os.path.join(directory, filename))
    image = np.array(image, dtype=np.uint8)
    images.append(image)

  return np.array(images)

if __name__ == "__main__":
  directory = "C:\Codes\glOrthoView\data\stack"
  #cwd = os.getcwd()  # Get the current working directory (cwd)
  #files = os.listdir(cwd)  # Get all the files in that directory
  #print("Files in %r: %s" % (cwd, files))
  images = load_images_uchar(directory)
  #print(images.shape)
