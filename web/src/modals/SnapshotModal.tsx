/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import {
    forwardRef,
    useCallback,
    useEffect,
    useImperativeHandle,
    useRef,
    useState
} from "react";
import { Button, Form, Modal, ModalProps, Select } from "react-daisyui";

import { createPortal } from "react-dom";
import axios from "axios";
import SectionSelect from "../components/SectionSelect";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faArrowDown, faCopy } from "@fortawesome/free-solid-svg-icons";

export type ISnapshotModalProps = ModalProps & {
    sections: Section[];
    sourceSection?: Section;
    targetSection?: Section;
};

export const SnapshotModal = forwardRef<HTMLDialogElement, ISnapshotModalProps>(
    (props, ref) => {
        const [sourceSection, setSourceSection] = useState<
            Section | undefined
        >();

        useEffect(() => {
            setSourceSection({
                ...props.sections.at(0),
                ...{ repository: undefined },
                ...props.sourceSection
            });
        }, [props.sourceSection, props.sections]);

        const [targetSection, setTargetSection] = useState<
            Section | undefined
        >();

        useEffect(() => {
            setTargetSection({
                ...props.sections.at(0),
                ...props.targetSection
            });
        }, [props.targetSection, props.sections, sourceSection]);

        useEffect(() => {
            setTargetSection({
                ...targetSection,
                repository: sourceSection?.repository,
                architecture: sourceSection?.architecture
            });
        }, [sourceSection]);

        const internalRef = useRef<HTMLDialogElement>(null);
        useImperativeHandle<HTMLDialogElement | null, HTMLDialogElement | null>(
            ref,
            () => internalRef.current
        );

        const doSnap = useCallback(async () => {
            if (
                !sourceSection?.branch ||
                !targetSection?.branch ||
                !sourceSection?.architecture
            ) {
                return;
            }
            try {
                await axios.post("/api/packages/snap/branch", {
                    sourceBranch: sourceSection.branch,
                    targetBranch: targetSection.branch,
                    architecture: sourceSection.architecture
                });
                internalRef?.current?.close();
            } catch (error) {}
        }, [sourceSection, targetSection, props.sections]);

        return createPortal(
            <Modal ref={internalRef} {...props}>
                <h1 className="text-3xl font-bold">
                    <FontAwesomeIcon className="pr-2" size="xs" icon={faCopy} />
                    Snapshot
                </h1>
                <Form>
                    <Form.Label title="Source section" />
                    <SectionSelect
                        disabled={[false, true, false]}
                        selectedSection={sourceSection}
                        sections={props.sections}
                        onSelected={(value) => {
                            setSourceSection(value);
                        }}
                    />
                </Form>
                <div className="flex justify-center my-4">
                    <FontAwesomeIcon icon={faArrowDown} className="h-6 w-6" />
                </div>

                <Form>
                    <Form.Label title="Target section" />
                    <SectionSelect
                        disabled={[false, true, true]}
                        selectedSection={targetSection}
                        sections={props.sections}
                        onSelected={(value) => {
                            setTargetSection(value);
                        }}
                    />
                </Form>
                <Form className="mt-6">
                    <span className="flex space-x-4 justify-end">
                        <Button
                            type="button"
                            color="primary"
                            onClick={(_e) => doSnap()}
                        >
                            Snap
                        </Button>
                    </span>
                </Form>
            </Modal>,
            document.body
        );
    }
);
export default SnapshotModal;
