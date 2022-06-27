window.addEventListener('message', event => {

    const message = event.data; // The JSON data our extension sent

    switch (message.command) {
        case 'test_server_ping_success':
            showConnectionSuccessStatus();
            break;
        case 'test_server_ping_failure':
            showConnectionFailureStatus();
            break;
        case 'check_server_ping_success':
            showConnectionSuccessStatus();
            nextStep();
            break;
        case 'check_server_ping_failure':
            showConnectionFailureStatus();
            openModal();

            break;
        default:
            console.log(`ERROR: Unkown message: ${message.command}`);
    }
});

const os = getVarValue("os");
console.log(`OS Platform: ${os}`);

if (os === 'win32') {
    removeElementByClassName('installer-tab');
    removeElementByClassName('installer-step');
}

let currentTabIndex = 0;
showTab(currentTabIndex);

function showTab(tabIndex) {
    const prevButton = document.getElementById("prevBtn");
    const nextButton = document.getElementById("nextBtn");

    const tabElements = document.getElementsByClassName("utbot-form__tab");
    const currentTabElement = tabElements[tabIndex];
    currentTabElement.classList.add('active');

    // Show previous button if neccesary 
    if (tabIndex == 0) {
        prevButton.classList.remove('active');
    } else {
        prevButton.classList.add('active');
    }

    // Set nextButton title according to tab number
    if (tabIndex == tabElements.length - 1) {
        nextButton.innerHTML = "Finish";
    } else {
        nextButton.innerHTML = "Next";
    }
    fixStepIndicator(tabIndex);
}

function nextButtonHandler() {
    const tab = document.getElementsByClassName("connection-tab");
    if (!tab || tab.length !== 1) {
        console.log("Something is wrong: connection-tab");
        return;
    }
    isConnectionTabActive = tab[0].classList.contains("active");
    if (isConnectionTabActive) {
        checkConnectionImplicitly();
    } else {
        nextStep();
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

function showConnectionSuccessStatus() {
    const connectionLoader = document.getElementsByClassName("utbot-form__tab_connection_loader")[0];
    const connectionSuccess = document.getElementsByClassName("utbot-form__tab_connection_success")[0];
    const connectionFailure = document.getElementsByClassName("utbot-form__tab_connection_failure")[0];

    connectionSuccess.classList.add('active');
    connectionLoader.classList.remove('active');
    connectionFailure.classList.remove('active');
}

function showConnectionFailureStatus() {
    const connectionLoader = document.getElementsByClassName("utbot-form__tab_connection_loader")[0];
    const connectionSuccess = document.getElementsByClassName("utbot-form__tab_connection_success")[0];
    const connectionFailure = document.getElementsByClassName("utbot-form__tab_connection_failure")[0];

    connectionFailure.classList.add('active');
    connectionLoader.classList.remove('active');
    connectionSuccess.classList.remove('active');
}

function showConnectionLoadingStatus() {
    const connectionLoader = document.getElementsByClassName("utbot-form__tab_connection_loader")[0];
    const connectionSuccess = document.getElementsByClassName("utbot-form__tab_connection_success")[0];
    const connectionFailure = document.getElementsByClassName("utbot-form__tab_connection_failure")[0];

    connectionLoader.classList.add('active');
    connectionFailure.classList.remove('active');
    connectionSuccess.classList.remove('active');
}


function nextPrev(tabShift) {
    if (tabShift !== 1 && tabShift !== -1) {
        return false;
    }
    let tabElements = document.getElementsByClassName("utbot-form__tab");
    let currentTabElement = tabElements[currentTabIndex];
    if (tabShift == 1 && !validateForm()) {
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
        if (tabInput.value == "") {
            tabInput.value = tabInput.placeholder;
        }
        if (tabInput.value == "" || !tabInput.checkValidity()) {
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
    const button = document.getElementById("runInstallerBtn");
    const messageBlock = document.getElementsByClassName("utbot-form__tab_installer_message")[0]
    if (button !== undefined && button !== null) {
        button.disabled = false;
    }
    if (messageBlock !== undefined && messageBlock !== null) {
        messageBlock.classList.remove("active");
    }

}

function getVarValue(name) {
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

/*
 * Communication with VSCode via API 
 */
const vscode = acquireVsCodeApi();

function sendPortAndHost() {
    const hostInputElement = document.getElementById('hostInput');
    const portInputElement = document.getElementById('portInput');
    vscode.postMessage({
        command: 'set_host_and_port',
        host: hostInputElement.value,
        port: parseInt(portInputElement.value)
    });
}

function sendMappingPath() {
    const mappingInputElement = document.getElementById('mappingInput');
    vscode.postMessage({
        command: 'set_mapping_path',
        mappingPath: mappingInputElement.value
    });
}

function sendBuildInfo() {
    const buildDirInputElement = document.getElementById('buildDirectory');
    const cmakeOptionsInputElement = document.getElementById('cmakeOptions');
    vscode.postMessage({
        command: 'set_build_info',
        buildDirectory: buildDirInputElement.value,
        cmakeOptions: cmakeOptionsInputElement.value
    });
}

/**
 * Request to test the connection explicitly (on button click). 
 */
function testConnection() {
    const hostInputElement = document.getElementById('hostInput');
    const portInputElement = document.getElementById('portInput');
    showConnectionLoadingStatus();
    vscode.postMessage({
        command: 'test_connection',
        host: hostInputElement.value,
        port: parseInt(portInputElement.value)
    });
}

/**
 * Request to run installation script. 
 */
function runInstallator() {
    const button = document.getElementById("runInstallerBtn");
    const messageBlock = document.getElementsByClassName("utbot-form__tab_installer_message")[0]
    button.disabled = true;
    messageBlock.classList.add("active");
    vscode.postMessage({
        command: 'run_installator'
    });
}

/**
 * Request to check the connection before proceeding to the next step. 
 */
function checkConnectionImplicitly() {
    const hostInputElement = document.getElementById('hostInput');
    const portInputElement = document.getElementById('portInput');
    showConnectionLoadingStatus();
    vscode.postMessage({
        command: 'check_connection',
        host: hostInputElement.value,
        port: parseInt(portInputElement.value)
    });
}

function closeWizard() {
    vscode.postMessage({ command: 'close_wizard' });
}
