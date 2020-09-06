%image to Game Boy tile converter, Raphaël BOICHOT 2020/08/31
%this file can be run with GNU Octake or Matlab
%just run this script with images into the folder "Images"
clc;
clear;
margin=3;% you can choose the number of blank lines between images here
imagefiles = dir('./Images/*.png');% the default format is png
nfiles = length(imagefiles);    % Number of files found
fid=fopen('Hex_data.txt','w');

for k=1:1:nfiles
currentfilename = imagefiles(k).name;
a=imread(['./Images/',currentfilename]);
disp(['Tiling image ',currentfilename,' in progress...'])
figure(1)
imagesc(a)
colormap gray;
[hauteur, largeur, profondeur]=size(a);
if not(rem(hauteur,16)==0);msgbox('The image height is not a multiple of 16 !');end
if not(largeur==160);msgbox('The image width is not 160 !');end
C = unique(a);
if not(length(C)==4); msgbox('The image is not 4 colors !');end
uni_tile=255*ones(8,8);
Black=C(1);
Dgray=C(2);
Lgray=C(3);
White=C(4);
hor_tile=largeur/8;
vert_tile=hauteur/8;
tile=0;
H=1;
L=1;
H_tile=1;
L_tile=1;
O=[];
total_tiles=hor_tile*vert_tile;
for x=1:1:hor_tile   
  for y=1:1:vert_tile   
    tile=tile+1;
    b=a((H:H+7),(L:L+7));
 
    for i=1:8
        for j=1:8
          
         if b(i,j)==Lgray;  V1(j)=('1'); V2(j)=('0');end;
         if b(i,j)==Dgray;  V1(j)=('0'); V2(j)=('1');end;
         if b(i,j)==White;  V1(j)=('0'); V2(j)=('0');end;
         if b(i,j)==Black;  V1(j)=('1'); V2(j)=('1');end;
     
        end
    O=[O,num2str(dec2hex(bin2dec(V1),2),2),' ',num2str(dec2hex(bin2dec(V2),2),2),' '];
    end
  rectangle('Position',[L-1 H-1 8 8],'EdgeColor','r');
    if tile==40
        O=O(1:end-1);
        fprintf(fid,O,'%s');
        fprintf(fid,'\n\r');
        length(O);
        O=[];
        tile=0;
    end
  
 drawnow
  
    L=L+8;
    L_tile=L_tile+1;
      if L>=largeur
      L=1;
      L_tile=1;
      H=H+8;
      H_tile=H_tile+1;  
    end

    end
end
blank=[];
for i=1:1:640;
    blank=[blank,'00 '];
end
blank=blank(1:1:end-1);
for i=1:1:margin
fprintf(fid,blank,'%s');
fprintf(fid,'\n\r');
end;
   
      
end
fclose(fid);
disp('End of conversion')