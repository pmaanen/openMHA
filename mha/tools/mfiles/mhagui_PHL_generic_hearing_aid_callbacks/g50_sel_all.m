function [handle,dat] = g50_sel_all(src,event)
handle = guidata(src);
dat = handle.dat;

edit_tag = get(gcbo,'tag');
max_all_right = get(handle.maxgain_all_edit_right,'String');
max_right = strsplit(max_all_right,' ');
max_all_left = get(handle.maxgain_all_edit_left,'String');
max_left = strsplit(max_all_left,' ');

dat.g50.ref_right = [handle.G50_right_slider1.Value handle.G50_right_slider2.Value handle.G50_right_slider3.Value handle.G50_right_slider4.Value handle.G50_right_slider5.Value];
dat.g50.max_right = max([handle.G50_right_slider1.Max-handle.G50_right_slider1.Value; handle.G50_right_slider2.Max-handle.G50_right_slider2.Value ;...
    handle.G50_right_slider3.Max-handle.G50_right_slider3.Value; handle.G50_right_slider4.Max-handle.G50_right_slider4.Value;...
    handle.G50_right_slider5.Max-handle.G50_right_slider5.Value]);
dat.g50.min_right = -max([handle.G50_right_slider1.Value-handle.G50_right_slider1.Min; handle.G50_right_slider2.Value-handle.G50_right_slider2.Min;...
    handle.G50_right_slider3.Value-handle.G50_right_slider3.Min; handle.G50_right_slider4.Value-handle.G50_right_slider4.Min;...
    handle.G50_right_slider5.Value-handle.G50_right_slider5.Min]);

dat.g50.ref_left = [handle.G50_left_slider1.Value handle.G50_left_slider2.Value handle.G50_left_slider3.Value handle.G50_left_slider4.Value handle.G50_left_slider5.Value];
dat.g50.max_left = max([handle.G50_left_slider1.Max-handle.G50_left_slider1.Value; handle.G50_left_slider2.Max-handle.G50_left_slider2.Value ;...
    handle.G50_left_slider3.Max-handle.G50_left_slider3.Value; handle.G50_left_slider4.Max-handle.G50_left_slider4.Value;...
    handle.G50_left_slider5.Max-handle.G50_left_slider5.Value]);
dat.g50.min_left = -max([handle.G50_left_slider1.Value-handle.G50_left_slider1.Min; handle.G50_left_slider2.Value-handle.G50_left_slider2.Min;...
    handle.G50_left_slider3.Value-handle.G50_left_slider3.Min; handle.G50_left_slider4.Value-handle.G50_left_slider4.Min;...
    handle.G50_left_slider5.Value-handle.G50_left_slider5.Min]);

switch edit_tag
    case 'right'
        if handle.sel_all_G50_right.Value == 0
            set([handle.G50_right_slider1 handle.G50_right_slider2 handle.G50_right_slider3 handle.G50_right_slider4 handle.G50_right_slider5],...
                    'backgroundcolor',[1 .5 .5]);
            if str2double(max_right(1)) ~= 0
                set([handle.G50_right_slider1 handle.G50_right_slider2 handle.G50_right_slider3 handle.G50_right_slider4 handle.G50_right_slider5],...
                    'enable','on');
                set([handle.G50_right_edit1 handle.G50_right_edit2 handle.G50_right_edit3 handle.G50_right_edit4 handle.G50_right_edit5],...
                    'enable','on');
            end
%             set(handle.G50_all_edit_right,'enable','off');
            set(handle.G50_all_slider_right ,'enable','off','backgroundcolor',[.5 .5 .5]);
        elseif handle.sel_all_G50_right.Value == 1
            set([handle.G50_right_slider1 handle.G50_right_slider2 handle.G50_right_slider3 handle.G50_right_slider4 handle.G50_right_slider5],...
                'enable','off','backgroundcolor',[.5 .5 .5]);
            set([handle.G50_right_edit1 handle.G50_right_edit2 handle.G50_right_edit3 handle.G50_right_edit4 handle.G50_right_edit5],...
                'enable','off');
            set(handle.G50_all_slider_right,'backgroundcolor',[1 .5 .5],'min',dat.g50.min_right,'max',dat.g50.max_right,...
                'value',0,'sliderstep',[1/(dat.g50.max_right-dat.g50.min_right) 3/(dat.g50.max_right-dat.g50.min_right)]);
            if str2double(max_right(1)) ~= 0
                set(handle.G50_all_slider_right,'enable','on');
%                 set(handle.G50_all_edit_right,'enable','on');
            end
        end
    case 'left'
        if handle.sel_all_G50_left.Value == 0
            set([handle.G50_left_slider1 handle.G50_left_slider2 handle.G50_left_slider3 handle.G50_left_slider4 handle.G50_left_slider5],...
                    'backgroundcolor',[.5 .5 1]);
            if str2double(max_left(1)) ~= 0
                set([handle.G50_left_slider1 handle.G50_left_slider2 handle.G50_left_slider3 handle.G50_left_slider4 handle.G50_left_slider5],...
                    'enable','on');
                set([handle.G50_left_edit1 handle.G50_left_edit2 handle.G50_left_edit3 handle.G50_left_edit4 handle.G50_left_edit5],...
                    'enable','on');
            end
%             set(handle.G50_all_edit_left,'enable','off');
            set(handle.G50_all_slider_left ,'enable','off','backgroundcolor',[.5 .5 .5]);
        elseif handle.sel_all_G50_left.Value == 1
            set([handle.G50_left_slider1 handle.G50_left_slider2 handle.G50_left_slider3 handle.G50_left_slider4 handle.G50_left_slider5],...
                'enable','off','backgroundcolor',[.5 .5 .5]);
            set([handle.G50_left_edit1 handle.G50_left_edit2 handle.G50_left_edit3 handle.G50_left_edit4 handle.G50_left_edit5],...
                'enable','off');
            set(handle.G50_all_slider_left,'backgroundcolor',[.5 .5 1],'min',dat.g50.min_left,'max',dat.g50.max_left,...
                'value',0,'sliderstep',[1/(dat.g50.max_left-dat.g50.min_left) 3/(dat.g50.max_left-dat.g50.min_left)]);
            if str2double(max_left(1)) ~= 0
                set(handle.G50_all_slider_left,'enable','on');
%                 set(handle.G50_all_edit_left,'enable','on');
            end
        end
end

handle.dat = dat;
guidata(src,handle);
