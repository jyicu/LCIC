close all;

% x=imread('./Kodak/kodim04.bmp');
x=imread('./lena.bmp');

code_idx = imread('idx.bmp');
code_idx = code_idx>0;
code_dir = imread('dir.bmp');
code_dir = code_dir>0;
code_hor = imread('hor.bmp');
code_hor = code_hor>0;

[H,W,C] = size(x);

R = int16(x(:,:,1));
G = int16(x(:,:,2));
B = int16(x(:,:,3));

Y=(R+2*G+B)/4;
U=R-G;
V=B-G;

T=3;

E=U(2:2:end,:);
O=U(1:2:end,:);

Xv = round(0.5*(E(2:end-1,2:end-1) + E(3:end,2:end-1)));
Xh = O(2:end-1,1:end-2);
Xo = O(2:end-1,2:end-1);

Pv = abs(Xo-Xv);
Ph = abs(Xo-Xh) + T;
idx = Pv>Ph;

num_dh = sum(idx(:));
prop1 = num_dh/length(idx(:))
cond = or(idx(1:end-2,2:end-1), idx(2:end-1,1:end-2));
idx2 = cond & idx(2:end-1,2:end-1);
num_ph = sum(idx2(:));
prop2 = num_ph/length(idx2(:))

figure();
imagesc(idx2)
figure
imagesc(code_idx)

% U=rot90(E,-1);
% % U=E;
% E=U(2:2:end,:);
% O=U(1:2:end,:);
% 
% Xv = round(0.5*(E(2:end-1,2:end-1) + E(3:end,2:end-1)));
% Xh = O(2:end-1,1:end-2);
% Xo = O(2:end-1,2:end-1);
% 
% Pv = abs(Xo-Xv);
% Ph = abs(Xo-Xh) + T;
% idx = Pv>Ph;
% 
% num_dh = sum(idx(:));
% prop1 = num_dh/length(idx(:))
% cond = idx(1:end-2,2:end-1) | idx(2:end-1,1:end-2);
% idx2 = cond & idx(2:end-1,2:end-1);
% num_ph = sum(idx2(:));
% prop2 = num_ph/length(idx2(:))
% 
% figure();
% imagesc(idx2)