%image to Game Boy tile converter, Raphaël BOICHOT 2021/09/03
%this file can be run with GNU Octave (ignore warnings) or Matlab
%just run this script with images into the folder "Images"
clc;
clear;
%-------------------------------------------------------------
margin=3;
INIT = [0x88 0x33 0x01 0x00 0x00 0x00 0x01 0x00 0x00 0x00]; %INT command
PRNT = [0x88 0x33 0x02 0x00 0x04 0x00 0x01 margin 0xE4 0x40];%, 0x2B, 0x01, 0x00, 0x00}; %PRINT without feed lines, default
INQU = [0x88 0x33 0x0F 0x00 0x00 0x00 0x0F 0x00 0x00 0x00]; %INQUIRY command
EMPT = [0x88 0x33 0x04 0x00 0x00 0x00 0x04 0x00 0x00 0x00]; %Empty data packet, mandatory for validate DATA packet
DATA = [0x88 0x33 0x04 0x00 0x80 0x02]; %DATA packet header, considering 640 bytes length by defaut (80 02), the footer is calculated onboard
%--------------------------------------------------------------
send_packet(INIT);
packets=0;
PRNT = add_checksum(PRNT);
imagefiles = dir('Images/*.png');% the default format is png, other are ignored
nfiles = length(imagefiles);    % Number of files found
fid=fopen('Hex_data.txt','w');

for k=1:1:nfiles
    currentfilename = imagefiles(k).name;
    a=imread(['Images/',currentfilename]);
    disp(['Converting image ',currentfilename,' in progress...'])
    figure(1)
    imshow(a)
    drawnow
    colormap gray;
    [hauteur, largeur, profondeur]=size(a);
    
    warning=0;
    
    if not(largeur==160);disp('Image rejected: width is not 160 !'); warning=1; end
    
    Black=-1;
    Dgray=-1;
    Lgray=-1;
    White=-1;
    
    C=unique(a);
    if length(C)>4;disp('Image rejected: contains more than 4 levels of color or gray !'); warning=1;end
    if length(C)==1;disp('Image rejected: empty image !'); warning=1;end
    
    if not(warning==1);
        
        if not(rem(hauteur,16)==0);
            disp('Image height is not a multiple of 16 : fixing image');
            C=unique(a);
            new_lines=ceil(hauteur/16)*16-hauteur;
            color_footer=double(C(end));
            footer=color_footer.*ones(new_lines,largeur, profondeur);
            a=[a;footer];
            imagesc(a)
            pause(0.25)
            [hauteur, largeur, profondeur]=size(a);
        end
        
        disp(['Transforming image ',currentfilename,' into GB tile data...'])
        switch length(C)
            case 4;
                Black=C(1);
                Dgray=C(2);
                Lgray=C(3);
                White=C(4);
            case 3;
                Black=C(1);
                Dgray=C(2);
                White=C(3);
            case 2;
                Black=C(1);
                White=C(2);
        end;
        
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
                        if b(i,j)==Lgray;  V1(j)=('1'); V2(j)=('0');end
                        if b(i,j)==Dgray;  V1(j)=('0'); V2(j)=('1');end
                        if b(i,j)==White;  V1(j)=('0'); V2(j)=('0');end
                        if b(i,j)==Black;  V1(j)=('1'); V2(j)=('1');end
                    end
                    O=[O,bin2dec(V1),bin2dec(V2)];
                end
                if tile==40
                    O=[];
                    tile=0;
                end
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
    end
end
fclose(fid);
disp('End of conversion')
