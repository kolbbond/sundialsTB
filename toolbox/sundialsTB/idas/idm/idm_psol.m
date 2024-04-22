function [z, flag, new_data] = idm_psol(t, yy, yp, rr, r, cj, fct, data)

%
% Wrapper around the actual user-provided Matlab function
%
global stop_flag;
if isempty(data)
  [z, flag] = feval(fct,t,yy,yp,rr,r,cj);
  new_data = [];
else
  [z, flag, new_data] = feval(fct,t,yy,yp,rr,r,cj,data);
  if(stop_flag)
      %flag = -1;
  end
end

