
count = 0;
for x = -256:255
   for p = -256:255
       sym = map(x,p);
       x_ = unmap(sym,p);
       if x ~= x_ || x>255 || x<-256 || sym<0 || sym>511
           disp('error')
           count = count +1;
       end
   end
end
if count == 0
    disp('PASS ALL!')
end

function sym = map(x,p)
    if p<0
        x = -x -1;
        p = -p -1;
    end
    
    if (x-p) >= -256
        sym = x - p;
    else
        sym = -x -1;
    end
    if sym <0
        sym = -2*sym -1;
    else
        sym = 2*sym;
    end
end

function x = unmap(sym,p)
    inv =0;
    if mod(sym,2) == 0
        sym =sym/2;
    else
        sym = -(sym+1)/2;
    end
    if p<0
        p = -p -1;
        inv =1;
    end
    
    if p +sym <256
        x = p+sym;
    else
        x = -sym -1;
    end
    if inv
        x = -x-1;
    end
end