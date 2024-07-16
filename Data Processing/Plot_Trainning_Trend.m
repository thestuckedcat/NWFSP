clear;
% get current .m file path
scriptFullPath = mfilename('fullpath');
% fetch directory of this .m
[scriptpath, ~,~] = fileparts(scriptFullPath);
% switch current path to here, otherwise it is in matlab mother folder
cd(scriptpath);




dataFolder = '../data';
resultFolder = '../result';

% dir means target is a directory, if exist return 7 otherwise 0
if ~exist(resultFolder,'dir')
    mkdir(resultFolder);
end

% fetch every txt file's data, fullfile
dataFiles = dir(fullfile(dataFolder,'*.txt'));
% Print debug information
disp(['Data folder: ', fullfile(pwd, dataFolder)]);
disp(['Result folder: ', fullfile(pwd, resultFolder)]);
disp(['Number of data files found: ', num2str(length(dataFiles))]);

% Define color for each data
colors = lines(length(dataFiles));

figure;
hold on;

for k = 1:length(dataFiles)
    % Fetch filename and path
    [~, fileName, ~] = fileparts(dataFiles(k).name);
    filePath = fullfile(dataFolder, dataFiles(k).name);
    
    % load data
    data = load(filePath);
    x = 1:length(data);
    
    %plot
    plot(x,data,'-','DisplayName',fileName,'Color',colors(k,:),'LineWidth',1.5,'MarkerSize',6);
end

% Customize plot for academic style

title('Data Trends', 'FontSize', 14);
xlabel('Index', 'FontSize', 12);
ylabel('Value', 'FontSize', 12);
legend('show', 'Location', 'best');
grid on;
set(gca, 'FontSize', 12);
hold off;
% Save the combined plot to result folder
saveas(gcf, fullfile(resultFolder, 'combined_plot.png'));

% Close the figure
close(gcf);