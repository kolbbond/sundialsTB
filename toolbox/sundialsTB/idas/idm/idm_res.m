function [res, flag, new_data] = idm_res(t, yy, yp, fct, data)

%
% Wrapper around the actual user-provided Matlab function
%

% kill button flag
global stop_flag;
%if(stop_flag)
    %return;
%end

if isempty(data)
  [res, flag] = feval(fct,t,yy,yp);
  new_data = [];
else
  [res, flag, new_data] = feval(fct,t,yy,yp,data);
   if(stop_flag)
        %flag = 1;
        return;
   end
end
