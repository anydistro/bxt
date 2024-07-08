import React, { forwardRef, useCallback } from "react";
import {
    CommitLogEntry,
    SyncLogEntry,
    DeployLogEntry,
    LogEntry
} from "../definitions/log";
import { Modal, ModalProps, Table } from "react-daisyui";
import { createPortal } from "react-dom";
import CommitDetails from "../components/logs/details/CommitDetails";
import DeployDetails from "../components/logs/details/DeployDetails";
import SyncDetails from "../components/logs/details/SyncDetails";

type LogEntryModalProps = ModalProps & {
    logData?: LogEntry;
};
const LogEntryModal = forwardRef<HTMLDialogElement, LogEntryModalProps>(
    ({ logData, ...props }, ref) => {
        const renderContent = useCallback(() => {
            switch (logData?.type) {
                case "Sync":
                    return (
                        <SyncDetails syncLogEntry={logData as SyncLogEntry} />
                    );
                case "Deploy":
                    return (
                        <DeployDetails
                            deployLogEntry={logData as DeployLogEntry}
                        />
                    );
                case "Commit":
                    return (
                        <CommitDetails
                            commitLogEntry={logData as CommitLogEntry}
                        />
                    );
            }
        }, [logData]);

        return createPortal(
            <Modal ref={ref} className="w-11/12 max-w-5xl" {...props}>
                {renderContent()}
            </Modal>,
            document.body
        );
    }
);
export default LogEntryModal;
