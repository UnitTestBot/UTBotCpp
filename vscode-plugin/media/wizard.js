window.addEventListener('message', event => {
    const message = event.data; // The JSON data our extension sent
    switch (message.command) {
        case 'test_connection_success':
            showConnectionSuccessStatus(message.clientVersion, message.serverVersion);
            break;
        case 'test_connection_failure':
            showConnectionFailureStatus();
            break;
        case 'check_connection_success':
            showConnectionSuccessStatus(message.clientVersion, message.serverVersion);
            nextStep();
            break;
        case 'check_connection_failure':
            showConnectionFailureStatus();
            openModal();
            break;
        default:
            console.log(`ERROR: Unknown message: ${message.command}`);
            break;
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

let nonLocalGRPCPort = getVarValue("defaultPort");
let nonLocalHost = getVarValue("sftpHost");
let nonLocalRemoteDir = getVarValue("sftpDir");
let currentTabIndex = 0;

function onStart() {
    DbgMessage("OnStart");
    const hostInput = $('hostInput');
    const portInput = $('portInput');
    const mappingInput = $('mappingInput');
    if (hostInput.value === "localhost"
        && portInput.value === getVarValue("defaultPort")
        && mappingInput.value === getVarValue("projectDir")) {
        $('useLocalHost').checked = true;
        hostInput.disabled = true;
        portInput.disabled = true;
        mappingInput.disabled = true;
    }
    if (os === 'win32') {
        removeElementByClassName('installer-tab');
        removeElementByClassName('installer-step');
    }
    showTab(currentTabIndex);
}

onStart();

function showTab(tabIndex) {
    const prevButton = $("prevBtn");
    const nextButton = $("nextBtn");

    const tabElements = document.getElementsByClassName("utbot-form__tab");
    const currentTabElement = tabElements[tabIndex];
    currentTabElement.classList.add('active');

    // Show previous button if necessary
    if (tabIndex === 0) {
        prevButton.classList.remove('active');
    } else {
        prevButton.classList.add('active');
    }

    // Set nextButton title according to tab number
    if (tabIndex === tabElements.length - 1) {
        nextButton.innerHTML = "Finish";
    } else {
        nextButton.innerHTML = "Next";
    }
    fixStepIndicator(tabIndex);
}

let checkHostTimer = null;
function showProgress(toShow) {
    showElement($("connection_loader"), toShow);
    if (toShow) {
        showElement($("connection_success"), false);
        showElement($("connection_warning"), false);
        showElement($("connection_failure"), false);
    }
}

function showElement(el, toShow) {
    if (toShow) {
        el.classList.add('active');
    }
    else {
        el.classList.remove('active');
    }
}

function restartCheckingConnection() {
    if (checkHostTimer != null) {
        clearTimeout(checkHostTimer);
        checkHostTimer = null;
        showProgress(false);
    }
    checkHostTimer = setTimeout(testConnection, 250, false);
    showProgress(true);
}

function isConnectionTab() {
    const tab = document.getElementsByClassName("connection-tab");
    if (!tab || tab.length !== 1) {
        console.log("Something is wrong: connection-tab");
        return;
    }
    return tab[0].classList.contains("active");
}

function nextButtonHandler() {
    if (isConnectionTab()) {
        testConnection(true);
    }
    else {
        nextStep();
        if (isConnectionTab()) {
            testConnection(false);
        }
    }
}

function prevButtonHandler() {
    prevStep();
}

function nextStep() {
    return nextPrev(1);
}

function prevStep() {
    return nextPrev(-1);
}

function showConnectionSuccessStatus(clientVersion, serverVersion) {
    showProgress(false);
    if (clientVersion !== serverVersion) {
        const connectionWarning = $("connection_warning");
        connectionWarning.innerText =
            connectionWarning.getAttribute("format").toString()
            + " "
            + `Server: ${serverVersion}, Client: ${clientVersion}`;
        showElement($("connection_success"), false);
        showElement(connectionWarning, true);
    }
    else {
        showElement($("connection_success"), true);
        showElement($("connection_warning"), false);
    }
    showElement($("connection_failure"), false);
}

function showConnectionFailureStatus() {
    showProgress(false);
    showElement($("connection_success"), false);
    showElement($("connection_warning"), false);
    showElement($("connection_failure"), true);
}

function showConnectionLoadingStatus() {
    showProgress(true);
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
    if (actionToPerform) {
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
}

function validateForm() {
    // This function deals with validation of the form fields
    let valid = true;
    let tabElements = document.getElementsByClassName("utbot-form__tab");
    let tabInputs = tabElements[currentTabIndex].getElementsByTagName("input");
    for (let i = 0; i < tabInputs.length; i++) {
        let tabInput = tabInputs[i];
        if (tabInput.value === "") {
            tabInput.value = tabInput.placeholder;
        }
        if (tabInput.value === "" || !tabInput.checkValidity()) {
            // TODO: custom validators
            tabInput.classList.add("invalid");
            valid = false;
        }
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

function closeModal() {
    let modal = document.getElementsByClassName("utbot-modal");
    if (!modal || modal.length !== 1) {
        console.log("Something is wrong, can't close modal");
        return;
    }
    modal[0].classList.remove("active");
}

function openModal() {
    let modal = document.getElementsByClassName("utbot-modal");
    if (!modal || modal.length !== 1) {
        console.log("Something is wrong, can't open modal");
        return;
    }
    modal[0].classList.add("active");
}

function closeModalAndGoToNextStep() {
    closeModal();
    nextStep();
}

function restoreOriginalStateOfSomeElements() {
    const button = $("runInstallerBtn");
    const messageBlock = document.getElementsByClassName("utbot-form__tab_installer_message")[0]
    if (button !== undefined && button !== null) {
        button.disabled = false;
    }
    if (messageBlock !== undefined && messageBlock !== null) {
        messageBlock.classList.remove("active");
    }
}

function getVarValue(name) {
    DbgMessage("var = " + document.getElementsByClassName("utbot-vars")[0].getAttribute(name));
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
    const mappingInput = $('mappingInput');
    const hostInput = $('hostInput');
    const portInput = $('portInput');
    const useLocalhost = $('useLocalHost').checked;
    if (useLocalhost) {
        DbgMessage("Local host is used!");

        nonLocalHost = hostInput.value;
        hostInput.value = "localhost";

        nonLocalGRPCPort = portInput.value;
        portInput.value = getVarValue("defaultPort");

        nonLocalRemoteDir = mappingInput.value;
        mappingInput.value = getVarValue("projectDir");
    }
    else {
        DbgMessage("Local host is not used!");
        hostInput.value = nonLocalHost;
        portInput.value = nonLocalGRPCPort;
        mappingInput.value = nonLocalRemoteDir;
    }
    mappingInput.disabled= useLocalhost;
    hostInput.disabled = useLocalhost;
    portInput.disabled = useLocalhost;

    restartCheckingConnection();
}

function sendServerSetup() {
    vscode.postMessage({
        command: 'set_server_setup',
        host: $('hostInput').value,
        port: parseInt($('portInput').value),
        mappingPath: $('mappingInput').value,
    });
}

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
function testConnection(withNextContinuation) {
    const hostInputElement = $('hostInput');
    const portInputElement = $('portInput');
    showProgress(true);
    vscode.postMessage({
        command: (withNextContinuation ? 'check_connection' : 'test_connection'),
        host: hostInputElement.value,
        port: parseInt(portInputElement.value)
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
    DbgMessage("openSFTPSettings");
    vscode.postMessage({
        command: 'openSFTPSettings',
        key: paramKey
    });
}