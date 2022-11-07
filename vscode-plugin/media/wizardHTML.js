
// external declaration in HTML
// const  GRPC_PREFIX =  "GRPC_";
// const  SFTP_PREFIX =  "SFTP_";

window.addEventListener('message', event => {
    const message = event.data; // The JSON data our extension sent
    //DbgMessage("addEventListener : " + message.command);
    if (message.command === "checkPlugins") {
        showPluginStatus("SFTP_", message.sftpPluginInstalled);
        showPluginStatus("SARIF_", message.sarifPluginInstalled);
        return;
    }
    let prefix = "";
    if (message.command.startsWith(GRPC_PREFIX)) {
        prefix = GRPC_PREFIX;
    }
    else if (message.command.startsWith(SFTP_PREFIX)) {
        prefix = SFTP_PREFIX;
    }
    //DbgMessage("addEventListener prefix : " + prefix);
    if (message.command.endsWith('test_connection_success')) {
        showConnectionSuccessStatus(prefix, message.clientVersion, message.serverVersion);
        //DbgMessage("addEventListener res : test_connection_success");
    }
    else if (message.command.endsWith('test_connection_failure')) {
        showConnectionFailureStatus(prefix);
        //DbgMessage("addEventListener res : test_connection_failure");
    }
    else {
        console.log(`ERROR: Unknown message: ${message.command}`);
    }
});

function $(id) {
    return document.getElementById(id);
}

/*
 * Communication with VSCode via API
 */
const vscode = acquireVsCodeApi();
function DbgMessage(message) {
     vscode.postMessage({
         command: 'dbg_message',
         message: message
     });
}

const os = getVarValue("os");
console.log(`OS Platform: ${os}`);

let nonLocalGRPCPort = getVarValue("defaultGRPCPort");
let nonLocalSFTPPort = getVarValue("defaultSFTPPort");
let nonLocalHost = getVarValue("serverHost");
let nonLocalRemoteDir = getVarValue("serverDir");
let currentTabIndex = 0;

function showProgress(prefix, toShow) {
    showElement($(prefix + "connection_loader"), toShow);
    if (toShow) {
        showElement($(prefix + "connection_success"), false);
        showElement($(prefix + "connection_warning"), false);
        showElement($(prefix + "connection_failure"), false);
    }
}

function showElement(el, toShow) {
    if (!el)
        return;

    if (toShow) {
        el.classList.add('active');
    }
    else {
        el.classList.remove('active');
    }
}

function isActive(el) {
    if (!el)
        return true;
    return el.classList.contains("active");
}

function isConnectionPortValid(el, prefix) {
    let valid = isActive($(prefix + "connection_success")) || isActive($(prefix + "connection_warning"))
    setValid(el, valid);
    return valid;
}


const mapTimerId = {};
function restartCheckingConnection(prefix, pause) {
    if (mapTimerId[prefix] != null) {
        clearTimeout(mapTimerId[prefix]);
        showProgress(prefix, false);        
    }
    mapTimerId[prefix] = setTimeout(testConnection, pause, prefix);
}

const pluginId = "plugin";
function restartCheckingPlugin(pause) {
    if (mapTimerId[pluginId] != null) {
        clearTimeout(mapTimerId[pluginId]);
    }
    mapTimerId[pluginId] = setTimeout(testPlugin, pause);
}

function testPlugin() {
    vscode.postMessage({
        command: 'check_plugins'
    });
}

function isTabWithClass(clazz) {
    const tab = document.getElementsByClassName(clazz);
    if (!tab || tab.length !== 1) {
        DbgMessage("Something is wrong: " + clazz);
        return false;
    }
    return isActive(tab[0]);
}

function isConnectionTab() {
    return isTabWithClass("connection-tab");
}

function isStartTab() {
    return isTabWithClass("start-tab");
}

function onStart() {
    const hostInput = $('hostInput');
    const portGRPCInput = $('portGRPCInput');
    const portSFTPInput = $('portSFTPInput');
    const serverPath = $('serverPath');

    // DbgMessage("hostInput.value : " + hostInput.value);
    // DbgMessage("portGRPCInput.value : " + portGRPCInput.value +  " " + getVarValue("defaultGRPCPort"));
    // DbgMessage("serverPath.value : " + serverPath.value +  " " + getVarValue("projectDir"));

    if (hostInput.value === "localhost"
        && portGRPCInput.value === getVarValue("defaultGRPCPort")
        && serverPath.value === getVarValue("projectDir")) {
        $('useLocalHost').checked = true;
        hostInput.disabled = true;
        portGRPCInput.disabled = true;

        portSFTPInput.disabled = true;
        portSFTPInput.value = "";

        serverPath.disabled = true;
    }
    if (os === 'win32') {
        removeElementByClassName('installer-tab');
        removeElementByClassName('installer-step');
    }
    restartCheckingPlugin(0);
    showTab(currentTabIndex);
}

onStart();

function showTab(tabIndex) {
    const prevButton = $("prevBtn");
    const nextButton = $("nextBtn");

    const tabElements = document.getElementsByClassName("utbot-form__tab");
    const currentTabElement = tabElements[tabIndex];

    showElement(currentTabElement, true);
    // Show previous button if necessary
    showElement(prevButton, tabIndex === 0);

    // Set nextButton title according to tab number
    if (tabIndex === tabElements.length - 1) {
        nextButton.innerHTML = "Finish";
    } else {
        nextButton.innerHTML = "Next";
    }
    fixStepIndicator(tabIndex);
}

function nextStep() {
    let res = nextPrev(1);
    if (isConnectionTab()) {
        restartCheckingConnection(GRPC_PREFIX, 0);
        restartCheckingConnection(SFTP_PREFIX, 0);
    } else if (isStartTab()) {
        restartCheckingPlugin(0);
    }
    return res;
}

function prevStep() {
    return nextPrev(-1);
}

function showConnectionSuccessStatus(prefix, clientVersion, serverVersion) {
    showProgress(prefix, false);
    if (clientVersion !== serverVersion) {
        const connectionWarning = $(prefix + "connection_warning");
        if (connectionWarning.getAttribute("format") !== null) {
            connectionWarning.innerText =
                connectionWarning.getAttribute("format").toString()
                + " "
                + `Server: ${serverVersion}, Client: ${clientVersion}`;
        }
        showElement($(prefix + "connection_success"), false);
        showElement(connectionWarning, true);
    }
    else {
        showElement($(prefix + "connection_success"), true);
        showElement($(prefix + "connection_warning"), false);
    }
    showElement($(prefix + "connection_failure"), false);
}

function showConnectionFailureStatus(prefix) {
    showProgress(prefix, false);
    showElement($(prefix + "connection_success"), false);
    showElement($(prefix + "connection_warning"), false);
    showElement($(prefix + "connection_failure"), true);
    // just ping if failed, maybe server down
    //restartCheckingConnection(prefix, 1500);
}

function showPluginStatus(prefix, installed) {
    showElement($(prefix + "installed"), installed);
    showElement($(prefix + "not_installed"), !installed);
    if (!installed) {
        restartCheckingPlugin(1000);
    }
}

function isPluginInstalledAndMarkValid(prefix) {
    let active = isActive($(prefix + "installed"));
    setValid($(prefix + "ref"), active);
    return active;
}

function nextPrev(tabShift) {
    if (tabShift !== 1 && tabShift !== -1) {
        return false;
    }
    let tabElements = document.getElementsByClassName("utbot-form__tab");
    let currentTabElement = tabElements[currentTabIndex];
    if (tabShift === 1 && !validateForm()) {
        return false;
    }
    let actionToPerform = currentTabElement.getAttribute("vs-message-callback");
    if (tabShift === 1 && actionToPerform) {
        // make setup action only on the way forward
        eval(actionToPerform)
    }
    currentTabElement.classList.remove('active');

    currentTabIndex = currentTabIndex + tabShift;
    if (currentTabIndex >= tabElements.length) {
        closeWizard();
        return false;
    }

    restoreOriginalStateOfSomeElements();
    showTab(currentTabIndex);
    return true;
}

function setValid(el, valid) {
    if (!el)
        return;

    if (!valid) {
        el.classList.add("invalid");
    } else {
        el.classList.remove("invalid");
    }
}

function checkInputValidAsNotEmpty(el, syncValidStatus) {
    if (!el)
        return;

    let valid = !!el.value;
    if (syncValidStatus) {
        setValid(el, valid);
    }
    return valid;
}

function defaultValidateForEmptyValue() {
    let valid = true;
    let tabElements = document.getElementsByClassName("utbot-form__tab");
    let tabInputs = tabElements[currentTabIndex].getElementsByTagName("input");
    for (let i = 0; i < tabInputs.length; i++) {
        valid &= checkInputValidAsNotEmpty(tabInputs[i], true);
    }
    return valid;
}

function validateForm() {
    let valid = true;
    if (isStartTab()) {
        // call them all - we need to mark all invalid input
        valid &= isPluginInstalledAndMarkValid("SFTP_")
        valid &= isPluginInstalledAndMarkValid("SARIF_");
    } else if (isConnectionTab()) {
        // call them all - we need to mark all invalid input
        valid &= isConnectionPortValid($("portGRPCInput"), GRPC_PREFIX);
        valid &= isConnectionPortValid($("portSFTPInput"), SFTP_PREFIX);
        valid &= checkInputValidAsNotEmpty($("hostInput"), true);
        valid &= checkInputValidAsNotEmpty($("serverPath"), true);
    } else {
        valid = this.defaultValidateForEmptyValue();
    }

    if (valid) {
        document.getElementsByClassName("utbot-form__steps_step")[currentTabIndex].classList.add("finish");
    }
    return valid;
}

function fixStepIndicator(tabIndex) {
    let steps = document.getElementsByClassName("utbot-form__steps_step");
    for (let i = 0; i < steps.length; i++) {
        steps[i].classList.remove("active");
        if (i > tabIndex) {
            steps[tabIndex].classList.remove('finish');
        }
    }
    steps[tabIndex].classList.add("active");
}

function restoreOriginalStateOfSomeElements() {
    const button = $("runInstallerBtn");
    const messageBlock = document.getElementsByClassName("utbot-form__tab_installer_message")[0];
    if (button !== undefined && button !== null) {
        button.disabled = false;
    }
    if (messageBlock !== undefined && messageBlock !== null) {
        messageBlock.classList.remove("active");
    }
}

function getVarValue(name) {
    //DbgMessage(name + " = " + document.getElementsByClassName("utbot-vars")[0].getAttribute(name));
    return document.getElementsByClassName("utbot-vars")[0].getAttribute(name);
}

function removeElementByClassName(className) {
    const removableElements = document.getElementsByClassName(className);
    for (let i = 0; i < removableElements.length; i++) {
        const parentNode = removableElements[i].parentNode;
        if (parentNode !== undefined || parentNode !== null) {
            parentNode.removeChild(removableElements[i]);
        }
    }
}

function handleOnOffDefaultConfigurationOnLocalhost() {
    const serverPath = $('serverPath');
    const hostInput = $('hostInput');
    const portGRPCInput = $('portGRPCInput');
    const portSFTPInput = $('portSFTPInput');
    const useLocalhost = $('useLocalHost').checked;
    if (useLocalhost) {
        DbgMessage("Local host is used!");

        nonLocalHost = hostInput.value;
        hostInput.value = "localhost";

        nonLocalGRPCPort = portGRPCInput.value;
        portGRPCInput.value = getVarValue("defaultGRPCPort");

        nonLocalSFTPPort  = portSFTPInput.value;
        portSFTPInput.value = ""; // Not Used

        nonLocalRemoteDir = serverPath.value;
        serverPath.value = getVarValue("projectDir");
    }
    else {
        DbgMessage("Local host is not used!");
        hostInput.value = nonLocalHost;
        portGRPCInput.value = nonLocalGRPCPort;
        portSFTPInput.value = nonLocalSFTPPort;
        serverPath.value = nonLocalRemoteDir;
    }
    serverPath.disabled= useLocalhost;
    hostInput.disabled = useLocalhost;
    portGRPCInput.disabled = useLocalhost;
    portSFTPInput.disabled = useLocalhost;

    restartCheckingConnection(GRPC_PREFIX, 0);
    restartCheckingConnection(SFTP_PREFIX, 0);
}

// user from eval
function sendServerSetup() {
    vscode.postMessage({
        command: 'set_server_setup',
        host: $('hostInput').value,
        portGRPC: parseInt($('portGRPCInput').value),
        portSFTP: parseInt($('portSFTPInput').value),
        serverPath: $('serverPath').value,
    });
}

// used from eval call
function sendBuildInfo() {
    const buildDirInputElement = $('buildDirectory');
    const cmakeOptionsInputElement = $('cmakeOptions');
    vscode.postMessage({
        command: 'set_build_info',
        buildDirectory: buildDirInputElement.value,
        cmakeOptions: cmakeOptionsInputElement.value
    });
}


/**
 * Request to test the connection with host
 */
function testConnection(prefix) {
    //DbgMessage(">>>>> testConnection " + prefix)
    let port = "";
    switch (prefix) {
        case GRPC_PREFIX:
            port = $('portGRPCInput').value;
            break;
        case SFTP_PREFIX:
            let el = $('portSFTPInput');
            if (!checkInputValidAsNotEmpty(el, false)) {
                // show `Not Used` connection status
                showProgress(prefix, true);
                showConnectionSuccessStatus(prefix, 1, -1);
                return;
            }
            port = el.value;
            break;
    }

    let letHostEl = $('hostInput');
    if (!checkInputValidAsNotEmpty(letHostEl, false)) {
        // show `Failed` connection status
        showProgress(prefix, true);
        showConnectionFailureStatus(prefix);
        return;
    }

    showProgress(prefix, true);
    vscode.postMessage({
        command: prefix + 'test_connection',
        host: letHostEl.value,
        port: parseInt(port)
    });
}

/**
 * Request to run installation script.
 */
function runInstaller() {
    const button = $("runInstallerBtn");
    const messageBlock = document.getElementsByClassName("utbot-form__tab_installer_message")[0]
    button.disabled = true;
    messageBlock.classList.add("active");
    vscode.postMessage({
        command: 'run_installer'
    });
}

function closeWizard() {
    vscode.postMessage({ command: 'close_wizard' });
}

function openSFTPSettings(paramKey) {
    //DbgMessage("openSFTPSettings");
    vscode.postMessage({
        command: 'openSFTPSettings',
        key: paramKey
    });
}