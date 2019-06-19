
count = 0;
for x = -256:255
   for p = -256:255
       sym = map(x,p);
       x_ = unmap(sym,p);
       if x ~= x_ || x>255 || x<-256 || sym<0 || sym>511
           disp('error')
           sym = map(x,p);
           x_ = unmap(sym,p);
           count = count +1;
       end
   end
end
if count == 0
    disp('PASS ALL!')
end

function sym = map(x,p)
    inv = 0;
    if p<0
        x = -x-1;
        p = -p-1;
        inv =1;
    end
    
    if inv
       if x>p
           sym = 2*(x-p) -1;
       elseif x< 2*p - 255
           sym = 255 -x;
       else
           sym = 2*(p-x);
       end
    else
       if x>=p
           sym = 2*(x-p);
       elseif x< 2*p - 255
           sym = 255 -x;
       else
           sym = 2*(p-x) -1;
       end
    end
end

function x = unmap(sym,p)
    inv = 0;
    if p<0
        p = -p-1;
        inv =1;
    end
    
    if inv
       if sym > 510 -2*p
           x = 255 -sym;
       elseif mod(sym,2) == 0
           x = p-sym/2;
       else
           x = p +(sym+1)/2;
       end
       x = -x -1;
    else
       if sym > 510 -2*p
           x = 255 -sym;
       elseif mod(sym,2) == 0
           x = p+sym/2;
       else
           x = p -(sym+1)/2;
       end
    end
    
end