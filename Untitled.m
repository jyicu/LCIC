x=imread('./Kodak/kodim05.bmp');
x=x(41:60,41:60,:);
imwrite(x,'test.bmp');
u = double(x(:,:,1)) - double(x(:,:,2));

y = (double(x(:,:,1)) + 2*double(x(:,:,2)) +double(x(:,:,3)))/4;
u_e=u(1:2:end,:,:);
u_o=u(2:2:end,:,:)

