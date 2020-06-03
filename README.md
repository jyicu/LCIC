LCIC
====

This is an implementation of "HIERARCHICAL PREDICTION AND CONTEXT ADAPTIVE CODING FOR LOSSLESS COLOR IMAGE COMPRESSION"

[[paper]](https://ieeexplore.ieee.org/document/6678216?arnumber=6678216) [[project page]](http://ispl.snu.ac.kr/light4u/project/LCIC/)

Requirement
-----------

This code requires jasper.exe https://www.ece.uvic.ca/~frodo/jasper/

Usage
-----

### Encode

> WSI_compress.exe e [input_image (bmp)] [output_codefile(bin)]

### Decode

> WSI_compress.exe d [input_codefile(bin)] [output_image(bmp)]

Citation
-----

If you use the work released here for your research, please cite this paper:

```
@article{kim2013hierarchical,
  title={Hierarchical prediction and context adaptive coding for lossless color image compression},
  author={Kim, Seyun and Cho, Nam Ik},
  journal={IEEE Transactions on image processing},
  volume={23},
  number={1},
  pages={445--449},
  year={2013},
  publisher={IEEE}
}
```

