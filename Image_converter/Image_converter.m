  ##PC to SD printer, Raphael BOICHOT 2023/08/26
  ##this must be run with GNU Octave
  ##just run this script with images into the folder "Images"
  clc;
  clear;
  close all

  disp('-----------------------------------------------------------')
  disp('|Beware, this code is for GNU Octave ONLY !!!             |')
  disp('-----------------------------------------------------------')

  pkg load image
  margin=3;% you can choose the number of blank lines between images here
  imagefiles_png = dir('Images/*.png');
  imagefiles_jpg = dir('Images/*.jpg');
  imagefiles_jpeg = dir('Images/*.jpeg');
  imagefiles_bmp = dir('Images/*.bmp');
  imagefiles = [imagefiles_png; imagefiles_jpg; imagefiles_jpeg; imagefiles_bmp];
  nfiles = length(imagefiles);    % Number of files found
  fid=fopen('Hex_data.txt','w');
  packets=0;

  for k=1:1:nfiles
    currentfilename = imagefiles(k).name;
    disp(['Converting image ',currentfilename,' in progress...'])
    [a,map]=imread(['Images/',currentfilename]);

    if not(isempty(map));##dealing with indexed images
      disp('Indexed image, converting to grayscale');
      a=ind2gray(a,map);
    end

    [height, width, layers]=size(a);
    if layers>1##dealing with color images
      disp('Color image, converting to grayscale');
      a=rgb2gray(a);
      [height, width, layers]=size(a);
    end
    C=unique(a);

    if (length(C)<=4 && height==160);##dealing with pixel perfect image, bad orientation
      disp('Bad orientation, image rotated');
      a=imrotate(a,270);
      [heigth, width,layers]=size(a);
    end

    if (length(C)<=4 && not(width==160));##dealing with pixel perfect upscaled/downscaled images
      disp('Image is 2 bpp or less, which is good, but bad size: fixing it');
      a=imresize(a,160/width,"nearest");
      [heigth, width,layers]=size(a);
    end

    if (length(C)>4 || not(width==160));##dealing with 8-bit images in general
      disp('8-bits image rectified and dithered with Bayer matrices');
      a=image_rectifier(a);
      [height, width, layers]=size(a);
    end

    if length(C)==1;##dealing with one color images
      disp('Empty image -> neutralization, will print full white');
      a=zeros(height, width);
    end

    if not(rem(height,16)==0);##Fixing images not multiple of 16 pixels
      disp('Image height is not a multiple of 16 : fixing image');
      C=unique(a);
      new_lines=ceil(height/16)*16-height;
      color_footer=double(C(end));
      footer=color_footer.*ones(new_lines,width, layers);
      a=[a;footer];
      [height, width, layers]=size(a);
    end

    [height, width, layers]=size(a);
    C=unique(a);
    disp(['Buffering image ',currentfilename,' into GB tile data...'])
    switch length(C)
      case 4;##4 colors, OK
        Black=C(1);
        Dgray=C(2);
        Lgray=C(3);
        White=C(4);
      case 3;##3 colors, sacrify LG (not well printed)
        Black=C(1);
        Dgray=C(2);
        Lgray=[];
        White=C(3);
      case 2;##2 colors, sacrify LG and DG
        Black=C(1)
        Dgray=[];
        Lgray=[];
        White=C(2);
      end

      hor_tile=width/8;
      vert_tile=height/8;
      tile=0;
      H=1;
      L=1;
      H_tile=1;
      L_tile=1;
      O = '';
      y_graph=0;
      total_tiles=hor_tile*vert_tile;
      for x=1:1:hor_tile
        for y=1:1:vert_tile
          tile=tile+1;
          b=a((H:H+7),(L:L+7));
          for i=1:8
            for j=1:8
              if b(i,j)==Lgray;  V1(j)=('1'); V2(j)=('0');end
              if b(i,j)==Dgray;  V1(j)=('0'); V2(j)=('1');end
              if b(i,j)==White;  V1(j)=('0'); V2(j)=('0');end
              if b(i,j)==Black;  V1(j)=('1'); V2(j)=('1');end
            end
            O = [O, dec2hex(bin2dec(V1), 2), ' ', dec2hex(bin2dec(V2), 2), ' '];
          end
          if tile==40
            imshow(a)
            colormap gray
            h=rectangle('Position',[1 y_graph 160-1 16],'EdgeColor','r', 'LineWidth',1,'FaceColor', [1, 0, 0]);
            drawnow
            y_graph=y_graph+16;
            O=O(1:end-1);
            disp(O)
            fprintf(fid,O,'%s');
            fprintf(fid,'\n\r');
            length(O);
            tile=0;
            O = '';
          end
          L=L+8;
          L_tile=L_tile+1;
          if L>=width
            L=1;
            L_tile=1;
            H=H+8;
            H_tile=H_tile+1;
          end
        end
      end

      packets=packets+3;
      imshow(a)
      drawnow
      ##--------printing loop-----------------------------
      blank='';
      for i=1:1:640;
        blank=[blank,'00 '];
      end
      blank=blank(1:1:end-1);
      for i=1:1:margin
        fprintf(fid,blank,'%s');
        fprintf(fid,'\n\r');
        ##---------------------------------------------------
      end
    end

    fclose(fid);
    disp('End of conversion')
    close all
